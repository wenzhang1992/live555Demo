#ifndef __GdImageLib_H__
#define __GdImageLib_H__

#include <memory.h>

namespace GdImageLib
{
	extern unsigned char IRImage_ColorTable[9][256][4];

    void Map16BitTo8Bit_S(unsigned short *psh16BitData, long lDataLen, unsigned char *pby8BitData);
	/*
	 * 将Y16数据转换成Y8数据
	 */
	void	Map16BitTo8Bit_u(unsigned short *psh16BitData, long lDataLen, unsigned char* pby8BitData, int nBrightness = 100, int nContrast = 100);

	/*
	 * 将Y16数据转换成Y8数据
	 */
	void	Map16BitTo8Bit(short* psh16BitData, int nDataLen, unsigned char* pby8BitData, int nBrightness = 100, int nContrast = 100);

	/*
	 * 将Y8数据转换成bmp文件
	 *
	QImage	ImageDataY8_To_Bmp(int nImageWidth, int nImageHeight, int nColorTableIndex, unsigned char* pImageDataY8);
	*/

	// 对无符号16位X图像数据进行两点校正
	void CorrectWithKB(const char* strKBFilePath, unsigned short *pImgData, long lWidth, long lHeight);

	// 对图像进行上下翻转
	void Flip(unsigned char *pImgData, long lWidth, long lHeight, unsigned char byPixelByteCount);

	// 对图像进行左右镜像
	void Mirror(unsigned char *pImgData, long lWidth, long lHeight, unsigned char byPixelByteCount);

	// 对图像进行上下翻转和左右镜像
	void FlipAndMirror(unsigned char *pImgData, long lWidth, long lHeight, unsigned char byPixelByteCount);

	
}

#endif // !__FileDataStream_H__
