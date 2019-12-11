#pragma once
#include "VideoCodec.h"
#include "Windows.h"
#include "Common.h"


class CRTPService
{
public:
	CRTPService();

	void AddData(NALUH264Packet *packet);

	void SetStatus(bool status);

	VideoPacket* GetPacket();

	~CRTPService();
private:
	bool m_bRunStatus = false;

	std::mutex m_lock;

	uint8_t *m_pNALUH264Buffer = nullptr;

	//��̨�̣߳�����H264�ķ������
	std::thread *m_stRtpPacketThread = nullptr;

	Queue_S<NALUH264Packet> *m_pOutputNaluQueue = nullptr;

	std::vector<uint8_t> m_svDataBuffer;	

	//��H264��NAL����RTP�������
	uint16_t m_uiFrameSquence = 0;

	Queue_S<VideoPacket> m_sqRtpPacketQueue;

	void RtpPacketGenerate();

	//��һH264�������
	void RtpPacketGenerate_Signal(NALUH264Packet *packet);

	//FUsģʽH264�������
	void RtpPacketGenerate_FUs(NALUH264Packet *packet);
};

