#include "VideoCodec.h"
#include <string>
#define __x86_64__
VideoEncodec::VideoEncodec()
{
}

VideoEncodec::~VideoEncodec()
{
    //Close();
}

bool VideoEncodec::InitEncodec(int width, int height, bool IsNetWork, const char *fileName)
{
    //CloseEncodec();
    av_register_all();

    m_bIsNetWork = IsNetWork;
    m_iOutputVideoWidth = width;
    m_iOutputVideoHeight = height;
    m_fileName = (*fileName);

    av_register_all();

    m_pFormatCtx = avformat_alloc_context();

    m_pOutFormat = av_guess_format(NULL, fileName, NULL);

    m_pFormatCtx->oformat = m_pOutFormat;

    //open file
    if (IsNetWork)
    {
        if (avio_open(&m_pFormatCtx->pb, fileName, AVIO_FLAG_READ_WRITE) < 0)
        {
            return false;
        }
    }

    m_pVideoSt = avformat_new_stream(m_pFormatCtx, 0);

    if (m_pVideoSt == NULL)
    {
        return false;
    }

    m_pCodecCtx = m_pVideoSt->codec;
    m_pCodecCtx->codec_id = m_pOutFormat->video_codec;
    m_pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    m_pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    m_pCodecCtx->width = m_iOutputVideoWidth;
    m_pCodecCtx->height = m_iOutputVideoHeight;
    m_pCodecCtx->bit_rate = 400000;
    m_pCodecCtx->gop_size = 250;
    m_pCodecCtx->time_base.num = 1;
    m_pCodecCtx->time_base.den = 25;
    m_pCodecCtx->qmin = 10;
    m_pCodecCtx->qmax = 51;
    m_pCodecCtx->max_b_frames = 0;
    m_pCodec = avcodec_find_encoder(m_pCodecCtx->codec_id);

    //使能头部写入SPS,PPS
    //m_pCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;

    if (!m_pCodec)
    {
        return false;
    }

    int ret = avcodec_open2(m_pCodecCtx, m_pCodec, NULL);
    if (ret < 0)
    {
        char info[1024];
        av_strerror(ret, info, 1024);
        delete[] info;
        return false;
    }

    m_pFrame = av_frame_alloc();

    m_iPictureSize = avpicture_get_size(m_pCodecCtx->pix_fmt, m_pCodecCtx->width, m_pCodecCtx->height);

    m_pictureBuffer = (uint8_t *)av_malloc_array(1, m_iPictureSize);

    avpicture_fill((AVPicture *)m_pFrame, m_pictureBuffer, m_pCodecCtx->pix_fmt, m_pCodecCtx->width, m_pCodecCtx->height);

    //wirte head
    //if (IsNetWork)
    // {
    ret = avformat_write_header(m_pFormatCtx, NULL);
    if (ret != AVSTREAM_INIT_IN_WRITE_HEADER)
    {
        return false;
    }

    //m_fileterCtx = av_bitstream_filter_init("h264_mp4toannexb");

    av_new_packet(&m_pkt, m_iPictureSize);

    m_iYSize = m_pCodecCtx->width * m_pCodecCtx->height;
}

bool VideoEncodec::WriteEncodecFrame(uint8_t *srcData, uint8_t *dstData, int &frameSize, int frameNum)
{
    memcpy(m_pictureBuffer, srcData, m_iPictureSize);

    av_new_packet(&m_pkt, m_iPictureSize);

    m_pVideoSt->time_base.den = 25;

    m_pVideoSt->time_base.num = 1;

    //Y
    m_pFrame->data[0] = m_pictureBuffer;
    //U
    m_pFrame->data[1] = m_pictureBuffer + m_iYSize;
    //V
    m_pFrame->data[2] = m_pictureBuffer + m_iYSize * 5 / 4;

    m_pFrame->pts = frameNum * (m_pVideoSt->time_base.den) / ((m_pVideoSt->time_base.num) * 25);

    m_pFrame->width = m_iOutputVideoWidth;

    m_pFrame->height = m_iOutputVideoHeight;

    m_pFrame->format = AV_PIX_FMT_YUV420P;

    //int got_picture = 0;

    avcodec_send_frame(m_pCodecCtx, m_pFrame);

    //int ret = avcodec_encode_video2(m_pCodecCtx, &m_pkt, m_pFrame, &got_picture);

    int ret = avcodec_receive_packet(m_pCodecCtx, &m_pkt);

    av_write_frame(m_pFormatCtx, &m_pkt);

	memcpy(dstData, m_pkt.data, m_pkt.size);

	frameSize = m_pkt.size;

    return true;
}

bool VideoEncodec::CloseEncodec()
{
    if (m_pFormatCtx == nullptr || m_pOutFormat == nullptr || m_pVideoSt == nullptr || m_pCodecCtx == nullptr || m_pCodec == nullptr || m_pictureBuffer == nullptr || m_pFrame == nullptr)
    {
        return true;
    }

    int ret = FlushEncoder(m_pFormatCtx, 0);

    //m_pFormatCtx->duration = m_iOutputFrameCount*(m_pVideoSt->time_base.den) / ((m_pVideoSt->time_base.num) * 25);

    av_write_trailer(m_pFormatCtx);

    if (m_pVideoSt)
    {
        avcodec_close(m_pVideoSt->codec);
        av_free(m_pFrame);
        av_free(m_pictureBuffer);
    }

    avio_close(m_pFormatCtx->pb);

    if (ret < 0)
    {
        return false;
    }

    avformat_free_context(m_pFormatCtx);

    return true;
}

bool VideoEncodec::InitNet(char *url)
{
    m_pUrl = url;

    avformat_alloc_output_context2(&m_pFormatCtxNet, NULL, "flv", m_pUrl);

    if (!m_pFormatCtxNet)
    {
        return false;
    }

    m_pOutputFormatNet = m_pFormatCtxNet->oformat;

    for (int i = 0; i < m_pFormatCtx->nb_streams; i++)
    {
        AVStream *stream = m_pFormatCtx->streams[i];

        m_pStreamNet = avformat_new_stream(m_pFormatCtxNet, stream->codec->codec);

        if (!m_pStreamNet)
        {
            return false;
        }

        int ret = avcodec_copy_context(m_pStreamNet->codec, stream->codec);

        if (ret < 0)
        {
            return false;
        }

        m_pStreamNet->codec->codec_tag = 0;

        if (m_pFormatCtxNet->oformat->flags & AVFMT_GLOBALHEADER)
            m_pStreamNet->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }

    if (!(m_pOutputFormatNet->flags & AVFMT_NOFILE))
    {
        int ret = avio_open(&m_pFormatCtxNet->pb, m_pUrl, AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            return false;
        }
    }

    int ret = avformat_write_header(m_pFormatCtxNet, NULL);

    if (ret < 0)
    {
        return false;
    }

    m_iStartTime = av_gettime();
}

bool VideoEncodec::WriteNetFrame(int frameNum)
{
    //获取输入的packet
    //int ret = av_read_frame(m_pFormatCtx, &m_pkt);

    //if (ret < 0)
    //{
    //    return false;
    // }

    int ret = 0;
    if (m_pkt.pts == AV_NOPTS_VALUE)
    {
        AVRational time_base = m_pVideoSt->time_base;

        int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(m_pVideoSt->r_frame_rate);

        m_pkt.pts = (double)(frameNum * calc_duration) / (double)(av_q2d(time_base) * AV_TIME_BASE);

        m_pkt.dts = m_pkt.pts;

        m_pkt.duration = (double)calc_duration / (double)(av_q2d(time_base) * AV_TIME_BASE);
    }

    AVRational time_base = m_pVideoSt->time_base;

    AVRational time_base_q = {1, AV_TIME_BASE};

    int64_t pts_time = av_rescale_q(m_pkt.dts, time_base, time_base_q);

    int64_t now_time = av_gettime() - m_iStartTime;

    if (pts_time > now_time)
        av_usleep(pts_time - now_time);

    m_pkt.pts = av_rescale_q_rnd(m_pkt.pts, m_pVideoSt->time_base, m_pStreamNet->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));

    m_pkt.dts = av_rescale_q_rnd(m_pkt.dts, m_pVideoSt->time_base, m_pStreamNet->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));

    m_pkt.duration = av_rescale_q(m_pkt.duration, m_pVideoSt->time_base, m_pStreamNet->time_base);

    m_pkt.pos = -1;

    ret = av_interleaved_write_frame(m_pFormatCtxNet, &m_pkt);

    if (ret < 0)
    {
        return false;
    }

    av_free_packet(&m_pkt);
}

VideoDecodec::VideoDecodec()
{
}

VideoDecodec::~VideoDecodec()
{
}

bool VideoDecodec::InitDecodec(char *fileName)
{
    avformat_network_init();

    av_register_all();

    m_pInputFormatCtx = avformat_alloc_context();

    int ret;

    //char url[] = "rtp://127.0.0.1:8554/vlc";

    //AVDictionary *opts = NULL;

    //av_dict_set(&opts, "buffer_size", "102400", 0);
    //av_dict_set(&opts, "rtsp_transport", "udp", 0);
    //av_dict_set(&opts, "stimeout", "2000000", 0);
    //av_dict_set(&opts, "max_delay", "500000", 0);

    //std::string filename = "E:\\bikes.flv";

    if ((ret = avformat_open_input(&m_pInputFormatCtx, fileName, NULL, NULL)) < 0)
    {
        char *errInfo = new char[1024];
        av_strerror(ret, errInfo, 1024);
        delete[] errInfo;
        return AVERROR(ret);
    }

    if ((ret = avformat_find_stream_info(m_pInputFormatCtx, NULL)) < 0)
    {
        return false;
    }

    av_dump_format(m_pInputFormatCtx, 0, fileName, 0);

    ret = av_find_best_stream(m_pInputFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);

    if (ret < 0)
    {
        return false;
    }
    else
    {
        m_iStreamIndex = ret;
        m_pStream = m_pInputFormatCtx->streams[m_iStreamIndex];
        m_pCodecCtx = m_pStream->codec;
        m_pCodec = avcodec_find_decoder(m_pCodecCtx->codec_id);
        if (!m_pCodec)
        {
            return false;
        }

        if ((ret = avcodec_open2(m_pCodecCtx, m_pCodec, NULL)) < 0)
        {
            return false;
        }
    }
    m_iInputVideoHeight = m_pCodecCtx->height;

    m_iInputVideoWidth = m_pCodecCtx->width;

    if (m_pInputFormatCtx->duration != AV_NOPTS_VALUE)
    {
        m_iTimeDuration = m_pInputFormatCtx->duration;

        m_iFrameCount = m_pStream->nb_frames;
    }

    //�������뻺����
    m_pFrame = av_frame_alloc();

    m_pBuffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P,
                                                        m_iInputVideoWidth, m_iInputVideoHeight));

    avpicture_fill((AVPicture *)(m_pFrame), m_pBuffer, AV_PIX_FMT_YUV420P, m_iInputVideoWidth, m_iInputVideoHeight);

    return true;
}

bool VideoDecodec::ReadEncodecFrame(uint8_t *data, uint8_t *h264Data, int &size, int &frameNum)
{
    int got_picture = 0;
    if (av_read_frame(m_pInputFormatCtx, &m_pkt) >= 0)
    {
        //����ԭʼH264����
        memcpy(h264Data, m_pkt.data, m_pkt.size);
        size = m_pkt.size;

        if (m_pkt.stream_index == m_iStreamIndex)
        {
            int ret = avcodec_decode_video2(m_pCodecCtx, m_pFrame, &got_picture, &m_pkt);

            if (ret < 0)
            {
                return false;
            }

            if (got_picture)
            {
                memcpy(data, m_pFrame->data[0], m_iInputVideoHeight * m_iInputVideoWidth);
                memcpy(data + m_iInputVideoHeight * m_iInputVideoWidth, m_pFrame->data[1], m_iInputVideoHeight * m_iInputVideoWidth / 4);
                memcpy(data + m_iInputVideoHeight * m_iInputVideoWidth + m_iInputVideoHeight * m_iInputVideoWidth / 4, m_pFrame->data[2], m_iInputVideoHeight * m_iInputVideoWidth / 4);
                frameNum++;
                return true;
            }
            else
            {
                return false;
            }
        }
    }

    return false;
}

bool VideoDecodec::CloseDecodec()
{
    av_frame_free(&m_pFrame);

    avcodec_close(m_pCodecCtx);

    avformat_free_context(m_pInputFormatCtx);

    av_free(m_pBuffer);

    return true;
}

int VideoDecodec::GetVideoHeight()
{
    return m_iInputVideoHeight;
}

int VideoDecodec::GetVideoWidth()
{
    return m_iInputVideoWidth;
}

int VideoEncodec::FlushEncoder(AVFormatContext *fmt_ctx, unsigned int stream_index)
{
    int ret;
    int got_frame;
    AVPacket enc_pkt;
    if (!(fmt_ctx->streams[stream_index]->codec->codec->capabilities &
          CODEC_CAP_DELAY))
        return 0;
    while (1)
    {
        enc_pkt.data = NULL;
        enc_pkt.size = 0;
        av_init_packet(&enc_pkt);
        ret = avcodec_encode_video2(fmt_ctx->streams[stream_index]->codec, &enc_pkt,
                                    NULL, &got_frame);
        av_frame_free(NULL);
        if (ret < 0)
            break;
        if (!got_frame)
        {
            ret = 0;
            break;
        }
        printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", enc_pkt.size);
        ret = av_write_frame(fmt_ctx, &enc_pkt);
        if (ret < 0)
            break;
    }
    return ret;
}

ImageChange::ImageChange()
{
}

ImageChange::ImageChange(int dstWidth, int dstHeight, int srcWidth, int srcHeight, AVPixelFormat dstFmt, AVPixelFormat srcFmt)
{
    m_iDstHeight = dstHeight;

    m_iDstWidth = dstWidth;

    m_iSrcHeight = srcHeight;

    m_iSrcWidth = srcWidth;

    m_dstFmt = dstFmt;

    m_srcFmt = srcFmt;

    m_pDstFrame = av_frame_alloc();

    m_pSrcFrame = av_frame_alloc();

    m_pSrcBuffer = new uint8_t[avpicture_get_size(m_srcFmt, srcWidth, srcHeight)];

    m_pDstBuffer = new uint8_t[avpicture_get_size(m_dstFmt, m_iDstWidth, m_iDstHeight)];

    avpicture_fill((AVPicture *)(m_pSrcFrame), m_pSrcBuffer, m_srcFmt, srcWidth, srcHeight);

    avpicture_fill((AVPicture *)(m_pDstFrame), m_pDstBuffer, m_dstFmt, m_iDstWidth, m_iDstHeight);

    m_convertCtx = sws_getContext(srcWidth, srcHeight, srcFmt, dstWidth, dstHeight, dstFmt, SWS_BICUBIC, NULL, NULL, NULL);
}

ImageChange::~ImageChange()
{
    delete[] m_pSrcBuffer;
    delete[] m_pDstBuffer;

    av_frame_free(&m_pDstFrame);

    av_frame_free(&m_pSrcFrame);
}

void ImageChange::ConvertFmt(unsigned char *pSrcData, unsigned char *pDstData)
{
    memcpy(m_pSrcBuffer, pSrcData, avpicture_get_size(m_srcFmt, m_iSrcWidth, m_iSrcHeight));

    sws_scale(m_convertCtx, (const uint8_t *const *)m_pSrcFrame->data, m_pSrcFrame->linesize, 0, m_iSrcHeight, m_pDstFrame->data, m_pDstFrame->linesize);

    memcpy(pDstData, m_pDstBuffer, avpicture_get_size(m_dstFmt, m_iDstWidth, m_iDstHeight));
}

