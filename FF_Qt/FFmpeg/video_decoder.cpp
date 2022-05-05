#include "video_decoder.h"
#include "QImage"
#include "memory"
#include "../image_info/image_info.h"
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}
VideoDecoder::VideoDecoder()
{
    format_context_ = nullptr;
    video_stream_id_ = 0;
    frame_time_ = 0;
    width_ = 0;
    height_ = 0;
    packet_ = nullptr;
    image_funcs_.set_max_size(200); //最多含200个缓存，约10秒缓存
}

VideoDecoder::~VideoDecoder()
{
    avformat_free_context(format_context_);
}

bool VideoDecoder::Init(AVFormatContext* av_context)
{
    video_stream_id_ = av_find_best_stream(av_context, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    //解码器上下文copy
    AVCodecParameters* codec_param = av_context->streams[video_stream_id_]->codecpar;
    if(!codec_param)
    {
        return false;
    }
    //创建编码器上下文
    AVCodecContext* codec_ctx = avcodec_alloc_context3(av_context->video_codec);
    if (!codec_ctx) {
        return false;
    }
   
    //复制编码参数到上下文中
    int ret = avcodec_parameters_to_context(codec_ctx, codec_param);
    if (ret != 0) 
    {
        return false;
    }
    time_base_ = codec_ctx->time_base;
    width_ = codec_ctx->width;
    height_ = codec_ctx->height;
    //打开编码器
    ret = avcodec_open2(codec_ctx, nullptr, nullptr);
    if (ret != 0) {
        return false;
    }
    sws_context_ = sws_getContext(width_, height_, codec_ctx->pix_fmt, width_, height_, AVPixelFormat::AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);
    if(!sws_context_)
    {
        return false;
    }
	return true;
}

void VideoDecoder::Run()
{
    packet_ = (AVPacket*)av_malloc(sizeof(AVPacket));
    while(av_read_frame(format_context_, packet_) >= 0)
    {
        if (packet_->stream_index == AVMEDIA_TYPE_VIDEO)
        {
            if (avcodec_send_packet(format_context_->streams[video_stream_id_]->codec, packet_) != 0)
            {
                continue;
            }

            AVFrame* frame = av_frame_alloc();
            if (avcodec_receive_frame(format_context_->streams[video_stream_id_]->codec, frame) != 0)
            {
                av_frame_free(&frame);
                continue;
            }
            //解码线程
            std::shared_ptr<QImage> output = std::make_shared<QImage>(width_, height_, QImage::Format_ARGB32);
			auto func = [=](std::shared_ptr<QImage> img_ptr)
            {
                int64_t timestamp = frame->best_effort_timestamp * av_q2d(time_base_) * 1000.0;
              
				return PostImageTask(sws_context_, frame, width_, height_, timestamp, std::move(img_ptr));
            };
            image_funcs_.push_back(ImageFunc(std::move(output),std::move(func)));
        }
    }
    av_free(packet_);
}

ImageInfo* VideoDecoder::PostImageTask(SwsContext* sws_context, AVFrame* frame, int width, int height, int64_t timestamp, std::shared_ptr<QImage> output)
{
    //kThreadVideoRender
    ImageInfo* image_info = nullptr;
    if (frame)
    {
        int output_line_size[4];

        av_image_fill_linesizes(output_line_size, AV_PIX_FMT_ARGB, width);
        uint8_t* output_dst[] = { output->bits() };

        sws_scale(sws_context, frame->data, frame->linesize, 0, height, output_dst, output_line_size);
        av_frame_free(&frame);
        image_info = new ImageInfo(timestamp, std::move(output));
        return image_info;
    }
    return nullptr;
}