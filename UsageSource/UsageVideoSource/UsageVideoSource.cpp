// UsageVideoSource.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "UsageVideoSource.h"

UsageVideoSource* UsageVideoSource::createNew(UsageEnvironment& env, char const* fileName, unsigned preferredFrameSize /* = 0 */, unsigned playTimePerFrame /* = 0 */)
{

}

UsageVideoSource::UsageVideoSource(UsageEnvironment& env, char *fileName, unsigned preferredFrameSize, unsigned playTimePerFrame)
	:ByteStreamFileSource(env,0,preferredFrameSize,playTimePerFrame)
{
	m_fileStream = new std::ifstream(fileName, std::ios::in | std::ios::binary);

	m_ucImageBufferY16 = new unsigned char[ImageWidth*ImageHeight];

	m_ucImageBufferY8 = new unsigned char[ImageWidth*ImageHeight];

	m_ucImageBufferYUV420 = new unsigned char[ImageWidth*ImageHeight*1.5];

	m_pImageChange = new ImageChange(ImageWidth, ImageHeight, ImageWidth, ImageHeight, AV_PIX_FMT_YUV420P, AV_PIX_FMT_GRAY8);
}

UsageVideoSource::~UsageVideoSource()
{

}

void UsageVideoSource::doGetNextFrame()
{
	m_fileStream->read((char*)m_ucImageBufferY16, ImageSize);

	fFrameSize = m_fileStream->gcount();

	if (fFrameSize != ImageSize)
	{
		return;
	}

	GdImageLib::Map16BitTo8Bit_u(reinterpret_cast<unsigned short*>(m_ucImageBufferY16), ImageSize, m_ucImageBufferY8);

	m_pImageChange->ConvertFmt(m_ucImageBufferY8, m_ucImageBufferYUV420);


}

