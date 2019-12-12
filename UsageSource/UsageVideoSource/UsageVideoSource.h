#pragma once
#ifndef __USAGE_VIDEO_SOURCE_H__
#define __USAGE_VIDEO_SOURCE_H__
#define DllExport _declspec(dllexport)

#include "stdafx.h"
#include "ByteStreamFileSource.hh"
#include "Common.h"
DllExport class UsageVideoSource : public FramedSource
{
public:
	static UsageVideoSource* createNew(UsageEnvironment& env, char * fileName, unsigned preferredFrameSize , unsigned playTimePerFrame );

	virtual void doGetNextFrame();

protected:
	UsageVideoSource(UsageEnvironment& env,char *fileName,unsigned preferredFrameSize, unsigned playTimePerFrame);

	~UsageVideoSource();
private:
	unsigned m_fPreferredFrameSize;

	unsigned m_fPlayTimePerFrame;

	char *m_fileName = nullptr;

	unsigned char *m_ucImageBufferY16 = nullptr;

	unsigned char *m_ucImageBufferY8 = nullptr;

	unsigned char *m_ucImageBufferYUV420 = nullptr;

	unsigned char *m_ucImageBufferH264 = nullptr;

	std::ifstream *m_fileStream = nullptr;

	int m_iFrameNum = 0;

	ImageChange *m_pImageChange = nullptr;

	VideoEncodec *m_pVideoEncodec = nullptr;

	unsigned fLastPlayTime;
};

#endif