#include "stdafx.h"
#include "RTPService.h"

CRTPService::CRTPService()
{
	m_pOutputNaluQueue = new Queue_S<NALUH264Packet>();

	m_pNALUH264Buffer = new uint8_t[ImageWidth*ImageHeight*1.5];

	//初始化时，开启后台线程进行处理

	SetStatus(true);

	m_stRtpPacketThread = new std::thread([&]()->void {
		RtpPacketGenerate();
	});
}

void CRTPService::AddData(NALUH264Packet *packet)
{
	packet->GetData(m_pNALUH264Buffer);

	for (int i = 0; i < packet->GetSize(); i++)
	{
		m_svDataBuffer.push_back(m_pNALUH264Buffer[i]);
	}

	std::vector<uint8_t>::iterator beginItem = m_svDataBuffer.begin();

	std::vector<uint8_t>::iterator startItem;

	bool bisFirst = true;

	while (beginItem != m_svDataBuffer.end())
	{
		//解析00 00 00 01的NALU
		if (((*beginItem) == 0x00 && (*(beginItem + 1) == 0x00) && (*(beginItem + 2) == 0x00) && (*(beginItem + 3) == 0x01)))
		{
			bisFirst = false;

			if ((beginItem != m_svDataBuffer.begin())&&(beginItem != (m_svDataBuffer.begin() + 1)))
			{
				uint8_t *temp = new uint8_t[(beginItem - startItem)];

				std::vector<uint8_t>::iterator item = m_svDataBuffer.begin();

				for (int i = 0; i < (beginItem - startItem); i++)
				{
					temp[i] = (*item);

					item++;
				}

				NALUH264Packet *packetTemp = new NALUH264Packet((beginItem - startItem),packet->GetTimeStamp());

				//设置当前包为 00 00 00 01类型的NAL
				packetTemp->SetStartType(false);

				packetTemp->PushData(temp, (beginItem - startItem));

				m_pOutputNaluQueue->Push(packetTemp);

				delete[] temp;

				m_svDataBuffer.erase(startItem, beginItem + 1);

				//进行删除操作后，迭代器启始失效
				beginItem = m_svDataBuffer.begin();
			}
			else
			{
				startItem = beginItem;

				beginItem++;
			}
		}
		else if (((*beginItem) == 0x00 && (*(beginItem + 1) == 0x00) && (*(beginItem + 2) == 0x00) && (*(beginItem + 3) == 0x00) && (*(beginItem + 4) == 0x01)))
		{
			bisFirst = false;

			if (beginItem != m_svDataBuffer.begin())
			{
				uint8_t *temp = new uint8_t[(beginItem - startItem)];

				std::vector<uint8_t>::iterator item = beginItem;

				for (int i = 0; i < (beginItem - startItem); i++)
				{
					temp[i] = (*(item));
					item++;
				}

				NALUH264Packet *packetTemp = new NALUH264Packet((beginItem - startItem),packet->GetTimeStamp());

				//设置当前包为00 00 00 00 01类型的NAL
				packetTemp->SetStartType(true);

				packetTemp->PushData(temp, (beginItem - startItem));

				m_pOutputNaluQueue->Push(packetTemp);

				delete[] temp;

				m_svDataBuffer.erase(startItem, beginItem + 1);

				//进行删除操作后，迭代器起始标志失效，进行重新获取
				beginItem = m_svDataBuffer.begin();
			}
			else
			{
				startItem = beginItem;
				beginItem++;
			}
		}
		else
		{
			if (bisFirst)
			{
				beginItem = m_svDataBuffer.erase(beginItem);
			}
			else
			{
				beginItem++;
			}
		}
	}

	delete packet;
}

void CRTPService::SetStatus(bool status)
{
	std::unique_lock<std::mutex> lock(m_lock);

	m_bRunStatus = status;
}

VideoPacket* CRTPService::GetPacket()
{
	std::unique_lock<std::mutex> lock(m_lock);

	return m_sqRtpPacketQueue.Get();
}

CRTPService::~CRTPService()
{
	SetStatus(false);

	m_stRtpPacketThread->join();

	delete m_pOutputNaluQueue;
}

void CRTPService::RtpPacketGenerate()
{
	while (m_bRunStatus)
	{
		if (m_pOutputNaluQueue->Size() != 0)
		{
			NALUH264Packet *h264Packet = m_pOutputNaluQueue->Get();

			if (h264Packet->GetSize() >= 1400)
			{
				RtpPacketGenerate_FUs(h264Packet);
			}
			else
			{
				RtpPacketGenerate_Signal(h264Packet);
			}

			delete h264Packet;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}
}

void CRTPService::RtpPacketGenerate_Signal(NALUH264Packet *packet)
{
	RTPPacket_Signal *rtpPacket = new RTPPacket_Signal();

	rtpPacket->header.V = 0x2;

	rtpPacket->header.P = 0x0;

	rtpPacket->header.X = 0x0;

	rtpPacket->header.CC = 0x0;

	rtpPacket->header.M = 0x0;

	rtpPacket->header.PT = 0x60;

	m_uiFrameSquence++;

	rtpPacket->header.seqNumber = m_uiFrameSquence;

	rtpPacket->header.timeStamp = packet->GetTimeStamp();

	rtpPacket->header.SSRC = 0x00000000;

	rtpPacket->header.CSRC = 0x00000000;

	uint8_t *buffer = new uint8_t[packet->GetSize()];

	packet->GetData(buffer);

	if (packet->GetStartType())
	{
		rtpPacket->data = new uint8_t[packet->GetSize() - 5];

		memcpy(rtpPacket->data, buffer + 5, packet->GetSize() - 5);

		uint8_t *data = new uint8_t[packet->GetSize() - 5 + 16];

		memcpy(data, &(rtpPacket->header), 16);

		memcpy(data + 16, rtpPacket->data, packet->GetSize() - 5);

		VideoPacket *videoPacket = new VideoPacket();

		videoPacket->size = packet->GetSize() + 16;

		videoPacket->data = data;

		m_sqRtpPacketQueue.Push(videoPacket);
	}
	else
	{
		rtpPacket->data = new uint8_t[packet->GetSize() - 4];

		memcpy(rtpPacket->data, buffer + 4, packet->GetSize() - 4);

		uint8_t *data = new uint8_t[packet->GetSize() - 4 + 16];

		memcpy(data, &(rtpPacket->header), 16);

		memcpy(data + 16, rtpPacket->data, packet->GetSize() - 4);

		VideoPacket *videoPacket = new VideoPacket();

		videoPacket->size = packet->GetSize() + 16;

		videoPacket->data = data;

		m_sqRtpPacketQueue.Push(videoPacket);
	}

	delete[] buffer;

	delete rtpPacket;
}

void CRTPService::RtpPacketGenerate_FUs(NALUH264Packet *packet)
{
	int bufferCount = packet->GetSize();

	bool isStart = true;

	uint8_t *buffer = new uint8_t[packet->GetSize()];

	packet->GetData(buffer);

	uint8_t NALHead;

	if (packet->GetStartType())
	{
		NALHead = buffer[5];
	}
	else
	{
		NALHead = buffer[4];
	}
	
	while (bufferCount >1400)
	{
		RTPPacket_FUs *rtpPacket = new RTPPacket_FUs();

		memset(&(rtpPacket->header), 0x00, 16);

		uint8_t *data = new uint8_t[1400];

		rtpPacket->header.V = 0x2;

		rtpPacket->header.P = 0x0;

		rtpPacket->header.X = 0x0;

		rtpPacket->header.CC = 0x0;

		rtpPacket->header.M = 0x0;

		rtpPacket->header.PT = 0x60;

		m_uiFrameSquence++;

		rtpPacket->header.seqNumber = m_uiFrameSquence;

		rtpPacket->header.timeStamp = packet->GetTimeStamp();

		rtpPacket->header.SSRC = 0x00000000;

		rtpPacket->header.CSRC = 0x00000000;

		//复制RTP头
		memcpy(data, &(rtpPacket->header), 16);

		memset(&(rtpPacket->indicator), 0x00, 1);

		memset(&(rtpPacket->fuheader), 0x00, 1);

		uint8_t *temp =  (uint8_t*)(&(rtpPacket->indicator));

		(*temp) = ((*temp) & 0xE0) | (NALHead & 0xE0);

		temp = (uint8_t*)(&(rtpPacket->fuheader));

		(*temp) = ((*temp) & 0x1F) | (NALHead & 0x1F);

		if (isStart)
		{
			rtpPacket->fuheader.S = 0x1;

			isStart = false;
		}
		else
		{
			rtpPacket->fuheader.S = 0x0;
		}

		rtpPacket->fuheader.E = 0x0;

		rtpPacket->fuheader.R = 0x0;

		rtpPacket->indicator.TYPE = 28;

		//复制FUs的Indicator
		memcpy(data + 16, &(rtpPacket->indicator), 1);
		//复制FUs的Header
		memcpy(data + 17, &(rtpPacket->fuheader), 1);
		//复制载荷数据
		if (packet->GetStartType())
		{
			memcpy(data + 18, buffer + (packet->GetSize() - bufferCount) + 5, 1400 - 18);
		}
		else
		{
			memcpy(data + 18, buffer + (packet->GetSize() - bufferCount) + 4, 1400 - 18);
		}
		
		bufferCount -= (1400 - 18);

		VideoPacket *videoPacket = new VideoPacket();

		videoPacket->size = 1400;

		videoPacket->data = data;

		m_sqRtpPacketQueue.Push(videoPacket);

		delete rtpPacket;
	}

	if (bufferCount > 0)
	{
		RTPPacket_FUs *rtpPacket = new RTPPacket_FUs();

		uint8_t *data = new uint8_t[bufferCount+18];

		rtpPacket->header.V = 0x2;

		rtpPacket->header.P = 0x0;

		rtpPacket->header.X = 0x0;

		rtpPacket->header.CC = 0x0;

		rtpPacket->header.M = 0x0;

		rtpPacket->header.PT = 0x60;

		m_uiFrameSquence++;

		rtpPacket->header.seqNumber = m_uiFrameSquence;

		rtpPacket->header.timeStamp = packet->GetTimeStamp();

		rtpPacket->header.SSRC = 0x00000000;

		rtpPacket->header.CSRC = 0x00000000;

		//复制RTP头
		memcpy(data, &(rtpPacket->header), 16);

		uint8_t *temp = (uint8_t*)(&(rtpPacket->indicator));

		(*temp) = ((*temp) & 0xE0) | (NALHead & 0xE0);

		temp = (uint8_t*)(&(rtpPacket->fuheader));

		(*temp) = ((*temp) & 0x1F) | (NALHead & 0x1F);

		if (isStart)
		{
			rtpPacket->fuheader.S = 0x1;

			isStart = false;
		}
		else
		{
			rtpPacket->fuheader.S = 0x0;
		}
		//包结束标志置位
		rtpPacket->fuheader.E = 0x1;

		rtpPacket->fuheader.R = 0x0;

		rtpPacket->indicator.TYPE = 28;

		//复制FUs的Indicator
		memcpy(data + 16, &(rtpPacket->indicator), 1);
		//复制FUs的Header
		memcpy(data + 17, &(rtpPacket->fuheader), 1);
		//复制载荷数据
		if (packet->GetStartType())
		{
			memcpy(data + 18, buffer + (packet->GetSize() - bufferCount) + 5, bufferCount);
		}
		else
		{
			memcpy(data + 18, buffer + (packet->GetSize() - bufferCount) + 4, bufferCount);
		}

		VideoPacket *videoPacket = new VideoPacket();

		videoPacket->size = bufferCount + 18;

		videoPacket->data = data;

		m_sqRtpPacketQueue.Push(videoPacket);

		delete rtpPacket;
	}

	delete[] buffer;
}
