#pragma once
#include <Windows.h>
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/opt.h"
#include "libavutil/time.h"
#include "libswscale/swscale.h"
}

class VideoDecodec;
class VideoEncodec;

class VideoDecodec
{
  public:
    VideoDecodec();

    ~VideoDecodec();

    /********************************视频解封装**************************************/

    bool InitDecodec(char *fileName = nullptr);

    bool ReadEncodecFrame(uint8_t *data, uint8_t *h264Data, int &size, int &frameNum);

    bool CloseDecodec();

    int GetVideoHeight();

    int GetVideoWidth();

  public:
    /********************************视频解封装**************************************/
    AVFormatContext *m_pInputFormatCtx = nullptr;

    AVStream *m_pStream = nullptr;

    AVCodecContext *m_pCodecCtx = nullptr;

    AVCodec *m_pCodec = nullptr;

    AVPacket m_pkt;

    AVFrame *m_pFrame;

    uint8_t *m_pBuffer = nullptr;

    int m_iStreamIndex;

    int m_iInputVideoWidth = 0;

    int m_iInputVideoHeight = 0;

    int m_iInputVideoFrameCount = 0;

    int m_iInputFps = 0;

    int m_iTimeDuration = 0;

    int m_iFrameCount = 0;
};

class VideoEncodec
{
  public:
    VideoEncodec();

    ~VideoEncodec();

    /********************************视频封装录制**************************************/
    bool InitEncodec(int width, int height, bool IsNetWork, const char *fileName = "test.h264");

    bool WriteEncodecFrame(uint8_t *srcData, uint8_t *dstData, int &frameSize, int frameNum);

    bool CloseEncodec();

    bool InitNet(char *url);

    bool WriteNetFrame(int frameNum);

  private:
    /********************************网络视频封装**************************************/
    AVFormatContext *m_pFormatCtxNet = nullptr;

    AVOutputFormat *m_pOutputFormatNet = nullptr;

    char *m_pUrl = nullptr;

    AVStream *m_pStreamNet = nullptr;

    int64_t m_iStartTime = 0;

    /********************************视频封装录制**************************************/

    AVFormatContext *m_pFormatCtx = nullptr;

    AVOutputFormat *m_pOutFormat = nullptr;

    AVStream *m_pVideoSt = nullptr;

    AVCodecContext *m_pCodecCtx = nullptr;

    AVCodec *m_pCodec = nullptr;

    AVPacket m_pkt;

    uint8_t *m_pictureBuffer = nullptr;

    AVFrame *m_pFrame = nullptr;

    AVBitStreamFilterContext *m_fileterCtx;

    bool m_bIsNetWork = false;

    char m_fileName;

    int m_iPictureSize = 0;

    int m_iYSize = 0;

    int m_iFrameCount = 0;

    int m_iOutputVideoWidth = 0;

    int m_iOutputVideoHeight = 0;

    int m_iOutputFrameCount = 0;

    int FlushEncoder(AVFormatContext *fmt_ctx, unsigned int stream_index);
};

class ImageChange
{
  public:
    ImageChange();

    ~ImageChange();

    ImageChange(int dstWidth, int dstHeight,
                int srcWidth, int srcHeight,
                AVPixelFormat dstFmt, AVPixelFormat srcFmt);

    void ConvertFmt(unsigned char *pSrcData, unsigned char *pDstData);

  private:
    AVFrame *m_pDstFrame = nullptr;
    AVFrame *m_pSrcFrame = nullptr;

    int m_iDstWidth = 0;
    int m_iDstHeight = 0;
    int m_iSrcWidth = 0;
    int m_iSrcHeight = 0;

    AVPixelFormat m_dstFmt;
    AVPixelFormat m_srcFmt;

    uint8_t *m_pSrcBuffer = nullptr;

    uint8_t *m_pDstBuffer = nullptr;

    SwsContext *m_convertCtx = nullptr;
};
