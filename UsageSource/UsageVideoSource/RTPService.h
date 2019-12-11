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

	//后台线程，处理H264的封包操作
	std::thread *m_stRtpPacketThread = nullptr;

	Queue_S<NALUH264Packet> *m_pOutputNaluQueue = nullptr;

	std::vector<uint8_t> m_svDataBuffer;	

	//对H264的NAL进行RTP封包处理
	uint16_t m_uiFrameSquence = 0;

	Queue_S<VideoPacket> m_sqRtpPacketQueue;

	void RtpPacketGenerate();

	//单一H264封包操作
	void RtpPacketGenerate_Signal(NALUH264Packet *packet);

	//FUs模式H264封包处理
	void RtpPacketGenerate_FUs(NALUH264Packet *packet);
};

