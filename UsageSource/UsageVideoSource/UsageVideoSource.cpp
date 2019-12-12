// UsageVideoSource.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "UsageVideoSource.h"
#include "GroupsockHelper.hh"
UsageVideoSource* UsageVideoSource::createNew(UsageEnvironment& env, char * fileName, unsigned preferredFrameSize /* = 0 */, unsigned playTimePerFrame /* = 0 */)
{
	UsageVideoSource *newSource = new UsageVideoSource(env, fileName, preferredFrameSize, playTimePerFrame);

	return newSource;
}

UsageVideoSource::UsageVideoSource(UsageEnvironment& env, char *fileName, unsigned preferredFrameSize, unsigned playTimePerFrame)
	:FramedSource(env)
{
	m_fileStream = new std::ifstream(fileName, std::ios::in | std::ios::binary);

	m_ucImageBufferY16 = new unsigned char[ImageWidth*ImageHeight];

	m_ucImageBufferY8 = new unsigned char[ImageWidth*ImageHeight];

	m_ucImageBufferYUV420 = new unsigned char[ImageWidth*ImageHeight*1.5];

	m_ucImageBufferH264 = new unsigned char[ImageWidth*ImageHeight*1.5];

	m_pImageChange = new ImageChange(ImageWidth, ImageHeight, ImageWidth, ImageHeight, AV_PIX_FMT_YUV420P, AV_PIX_FMT_GRAY8);

	m_pVideoEncodec = new VideoEncodec();

	m_pVideoEncodec->InitEncodec(ImageWidth, ImageHeight, true);
}

UsageVideoSource::~UsageVideoSource()
{

}

void UsageVideoSource::doGetNextFrame()
{
	static int frameNum = 0;

	this->fMaxSize = ImageSize;

	m_fileStream->read((char*)m_ucImageBufferY16, ImageSize);

	int count = m_fileStream->gcount();

	if (count != ImageSize)
	{
		return;
	}

	GdImageLib::Map16BitTo8Bit_u(reinterpret_cast<unsigned short*>(m_ucImageBufferY16), ImageSize, m_ucImageBufferY8);

	m_pImageChange->ConvertFmt(m_ucImageBufferY8, m_ucImageBufferYUV420);

	m_pVideoEncodec->WriteEncodecFrame(m_ucImageBufferYUV420, m_ucImageBufferH264, this->fFrameSize, frameNum);

	if (m_fPlayTimePerFrame > 0 && m_fPreferredFrameSize > 0)
	{
		if (fPresentationTime.tv_sec == 0 && fPresentationTime.tv_usec == 0)
		{
			gettimeofday(&fPresentationTime, NULL);
		}
		else
		{
			unsigned uSeconds = fPresentationTime.tv_usec + fLastPlayTime;
			fPresentationTime.tv_sec += uSeconds / 1000000;
			fPresentationTime.tv_usec = uSeconds % 1000000;
		}

		fLastPlayTime = (m_fPlayTimePerFrame * fFrameSize)/(fFrameSize);
		fDurationInMicroseconds = fLastPlayTime;
	}
	else
	{
		gettimeofday(&fPresentationTime, NULL);
	}


	nextTask() = envir().taskScheduler().scheduleDelayedTask(0,
		(TaskFunc*)FramedSource::afterGetting, this);

	FramedSource::afterGetting(this);
}

