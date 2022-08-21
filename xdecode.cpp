#include "xdecode.h"
#include "qdebug.h"

xDecode::xDecode()
{

}

xDecode::xDecode(const QString url)
{
    m_url=url;
}

//采集开始前初始化
bool xDecode::begin()
{
    avformat_network_init();   //初始化FFmpeg网络模块
    av_register_all();         //初始化FFMPEG  调用了这个才能正常适用编码器和解码器

    //Allocate an AVFormatContext.
    i_fmt_ctx = avformat_alloc_context();
    AVDictionary *avdic=NULL;
    char option_key[]="rtsp_transport";
    char option_value[]="tcp";
    av_dict_set(&avdic,option_key,option_value,0);
    char option_key2[]="max_delay";
    char option_value2[]="100";
    av_dict_set(&avdic,option_key2,option_value2,0);

    if (avformat_open_input(&i_fmt_ctx, m_url.toStdString().data(), NULL, NULL) != 0) {
        qDebug()<<"can't open the file"<<endl;
        return false;
    }

    if (avformat_find_stream_info(i_fmt_ctx, NULL) < 0) {
        qDebug()<<"Could't find stream infomation."<<endl;
        return false;
    }

    videoStream = -1;
    ///循环查找视频中包含的流信息，直到找到视频类型的流
    ///便将其记录下来 保存到videoStream变量中
    ///这里我们现在只处理视频流  音频流先不管他
    for (uint32_t i = 0; i < i_fmt_ctx->nb_streams; i++) {
        if (i_fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
        }
    }

    ///如果videoStream为-1 说明没有找到视频流
    if (videoStream == -1) {
        qDebug()<<"Didn't find a video stream."<<endl;
        return false;
    }
    i_video_stream = i_fmt_ctx->streams[videoStream];

    ///查找解码器
    pCodecCtx = i_fmt_ctx->streams[videoStream]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    pCodecCtx->bit_rate =0;   //初始化为0
    pCodecCtx->time_base.num= 1;
    pCodecCtx->time_base.den= 25;
    pCodecCtx->frame_number =1;  //每包一个视频帧
    if (pCodec == NULL) {
        qDebug()<<"Codec not found"<<endl;
        return false;
    }
#if 1
    //打开解码器
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        qDebug()<<"Could not open codec"<<endl;
        return false;
    }

    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();

    //这里我们改成了 将解码后的YUV数据转换成RGB32
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height ,pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
                                     /*AV_PIX_FMT_RGB32*/AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
    numBytes = avpicture_get_size(/*AV_PIX_FMT_RGB32*/AV_PIX_FMT_YUV420P, pCodecCtx->width,pCodecCtx->height);

    out_buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    avpicture_fill((AVPicture *) pFrameRGB, out_buffer, /*AV_PIX_FMT_RGB32*/AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
#endif


    m_OutFile = fopen("rtsp.h264","ab");

#if 0
    const char *url="rtsp.h264";
    avformat_alloc_output_context2(&o_fmt_ctx, NULL, NULL, url);
    o_fmt = o_fmt_ctx->oformat;
    o_video_stream = avformat_new_stream(o_fmt_ctx, i_video_stream->codec->codec);

    int ret = avcodec_copy_context(o_video_stream->codec, i_video_stream->codec);
    if (ret < 0)
    {
        printf("Failed to copy context from input to output stream codec context\n");
        return false;
    }

    if (o_fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
    {
        o_video_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    av_dump_format(o_fmt_ctx, 0, url, 1);

    //Open output URL
    if (!(o_fmt->flags & AVFMT_NOFILE))
    {
        ret = avio_open(&o_fmt_ctx->pb, url, AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            printf("Could not open output URL '%s'", url);
            return false;
        }
    }
    avformat_write_header(o_fmt_ctx, NULL);
#else
    AVCodec* encodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    //AVCodec* encodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (!encodec)
    {
        qDebug() << "encodec == NULL" << endl;
    }
    OutCodecCtx = avcodec_alloc_context3(encodec);
    if (!OutCodecCtx)
    {
        qDebug() << "enc == NULL" << endl;
    }

#if 1
    //OutCodecCtx->bit_rate = 400000;
    //OutCodecCtx->width = pCodecCtx->width;
    //OutCodecCtx->height = pCodecCtx->height;
    //OutCodecCtx->time_base.num = 1;
    //OutCodecCtx->framerate = {25, 1};
    //OutCodecCtx->gop_size = 10;
    //OutCodecCtx->max_b_frames = 1;
    //OutCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

    OutCodecCtx->time_base.num = 1;
    OutCodecCtx->frame_number = 1; //每包一个视频帧
    OutCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    OutCodecCtx->bit_rate = 0;
    OutCodecCtx->time_base.den = 25;//帧率
    OutCodecCtx->width = 1280;//视频宽
    OutCodecCtx->height = 720;//视频高
    OutCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;


    //加载预设
    av_opt_set(OutCodecCtx->priv_data, "preset", "slow", 0);
    av_opt_set(OutCodecCtx->priv_data, "tune", "zerolatency", 0);
#endif
    int ret = avcodec_open2(OutCodecCtx, encodec,NULL);
    if (ret < 0)
    {
        qDebug() << "encodec open error" << endl;
        return false;
    }
#endif

    printf("---------------- File Information ---------------\n");
    av_dump_format(i_fmt_ctx, 0, m_url.toStdString().data(), 0);
    printf("-------------------------------------------------\n");
    return true;
}
bool xDecode::samp(QImage &img)
{
    int ret=0;
    int y_size = pCodecCtx->width * pCodecCtx->height;
    AVPacket* packet = (AVPacket *) malloc(sizeof(AVPacket));
    av_new_packet(packet, y_size);
    packet->size = 0;
    packet->data = NULL;

    if (av_read_frame(i_fmt_ctx, packet) < 0)
    {
        return false;
    }
    if (packet->stream_index == videoStream)
    {
        frame_index++;
        qDebug()<<"receive RTSP:"<<frame_index<< "size:"<<packet->size<<endl;

        pFrameRGB->format = AV_PIX_FMT_YUV420P;
        pFrameRGB->width = pCodecCtx->width;
        pFrameRGB->height = pCodecCtx->height;
        ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture,packet);
        if (ret < 0)
        {
            qDebug()<<"decode error"<<endl;
            return false;
        }

        if (got_picture)
        {
            sws_scale(img_convert_ctx,
                      (uint8_t const * const *) pFrame->data,
                      pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data,
                      pFrameRGB->linesize);
            //img =QImage((uchar *)out_buffer,pCodecCtx->width,pCodecCtx->height,QImage::Format_RGB32);
    #if 0
            if (last_pts == 0)
            {
                last_pts = packet->pts;
                last_dts = packet->dts;
            }
            else
            {
                packet->pts += last_pts;
                packet->dts += last_dts;
            }
            if (packet->dts < packet->pts)
            {
                packet->dts = packet->pts;
            }
            i_video_stream = i_fmt_ctx->streams[packet->stream_index];
            o_video_stream = o_fmt_ctx->streams[packet->stream_index];
            packet->duration = av_rescale_q(packet->duration, i_video_stream->time_base, i_video_stream->time_base);
            packet->pos = -1;
            av_interleaved_write_frame(o_fmt_ctx, packet);
    #else
            encode(OutCodecCtx, pFrameRGB, m_OutFile); //编码yuv
    #endif
        }


        if (frame_index > MAX_FRAMES)
        {
            av_free_packet(packet);
            return false;
        }
    }
    av_free_packet(packet);
    return true;
}
void xDecode::encode(AVCodecContext* enc_ctx, AVFrame* frame, FILE* outfile)
{
    int ret;
    AVPacket* pkt = av_packet_alloc();
    /* send the frame to the encoder */
    if (frame)
        printf("Send frame %d", frame->pts);

    ret = avcodec_send_frame(enc_ctx, frame);
    if (ret < 0)
    {
        fprintf(stderr, "Error sending a frame for encoding\n");
        return;
    }

    while (ret >= 0)
    {
        ret = avcodec_receive_packet(enc_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0)
        {
            fprintf(stderr, "Error during encoding\n");
            return;
        }

        printf("Write packet %d size=%d \n", pkt->size, pkt->pts);
        fwrite(pkt->data, 1, pkt->size, outfile);
        av_packet_unref(pkt);
    }
    av_packet_free(&pkt);
}

void xDecode::end()
{
    //av_write_trailer(o_fmt_ctx);
    avio_close(o_fmt_ctx->pb);
    avformat_free_context(o_fmt_ctx);
    av_free(out_buffer);
    av_free(pFrameRGB);
    avcodec_close(pCodecCtx);
    avformat_close_input(&i_fmt_ctx);
    fclose(m_OutFile);
    qDebug("xuzhekui OUT");
}
