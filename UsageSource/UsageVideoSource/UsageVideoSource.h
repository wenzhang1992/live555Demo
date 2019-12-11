#pragma once
#ifndef __USAGE_VIDEO_SOURCE_H__
#define __USAGE_VIDEO_SOURCE_H__
#define DllExport _declspec(dllexport)

#include "stdafx.h"
#include "ByteStreamFileSource.hh"
#include "Common.h"
DllExport class UsageVideoSource : public ByteStreamFileSource
{
public:
	static UsageVideoSource* createNew(UsageEnvironment& env, char const* fileName, unsigned preferredFrameSize /* = 0 */, unsigned playTimePerFrame /* = 0 */);

protected:
	UsageVideoSource(UsageEnvironment& env,char *fileName,unsigned preferredFrameSize, unsigned playTimePerFrame);

	~UsageVideoSource();
private:
	char *m_fileName = nullptr;

	unsigned char *m_ucImageBufferY16 = nullptr;

	unsigned char *m_ucImageBufferY8 = nullptr;

	unsigned char *m_ucImageBufferYUV420 = nullptr;

	std::ifstream *m_fileStream = nullptr;

	virtual void doGetNextFrame();

	int m_iFrameNum = 0;
};

#endif