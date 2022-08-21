#ifndef XDECODE_H
#define XDECODE_H
#include "QString"
#include "QImage"


extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include "libavfilter/avfilter.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/avutil.h"
#include "libavutil/pixfmt.h"
#include "libavutil/error.h"
#include "libavutil/imgutils.h"
}


class xDecode
{
public:
    xDecode();
    xDecode(const QString url);
    bool begin(); //初始化
    bool samp(QImage &img); //采集函数
    void end();  //结束
    void encode(AVCodecContext* enc_ctx, AVFrame* frame, FILE* outfile);
private:
    QString m_url;
    AVFormatContext *i_fmt_ctx;
    //AVPacket *packet;
    AVCodecContext *pCodecCtx;
    AVCodecContext *OutCodecCtx;
    AVCodec *pCodec;
    AVFrame *pFrame, *pFrameRGB;
    struct SwsContext *img_convert_ctx;
    int videoStream, got_picture;
    uint8_t *out_buffer;
    FILE* m_OutFile;
    int  numBytes;
    AVFormatContext *o_fmt_ctx;
    AVOutputFormat* o_fmt;
    AVStream *o_video_stream;
    AVStream *i_video_stream;
    int last_pts = 0;
    int last_dts = 0;
    int frame_index;
    static constexpr uint32_t MAX_FRAMES{100};
};

#endif // XDECODE_H
