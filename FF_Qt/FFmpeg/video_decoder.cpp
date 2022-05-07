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
    codec_context_ = nullptr;
    video_stream_id_ = 0;
    frame_time_ = 0;
    width_ = 0;
    height_ = 0;
    packet_ = nullptr;
    image_funcs_.set_max_size(200); //最多含200个缓存，约10秒缓存
}

VideoDecoder::~VideoDecoder()
{
    avformat_free_context(decoder_);
}

bool VideoDecoder::Init(const std::string& path)
{
    bool b_success = PrepareDeocde(path);
    if(!b_success)
    {
        return b_success;
    }
    video_stream_id_ = av_find_best_stream(decoder_, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    //解码器上下文copy
    AVCodecParameters* codec_param = decoder_->streams[video_stream_id_]->codecpar;
    if (!codec_param)
    {
        return false;
    }
    //创建编码器上下文
    codec_context_ = avcodec_alloc_context3(decoder_->video_codec);
    if (!codec_context_) {
        return false;
    }

    //复制编码参数到上下文中
    int ret = avcodec_parameters_to_context(codec_context_, codec_param);
    if (ret != 0)
    {
        return false;
    }
    
    width_ = codec_context_->width;
    height_ = codec_context_->height;
    frame_time_ = 1000.0 / decoder_->streams[video_stream_id_]->avg_frame_rate.num;
    //打开编码器
    auto codec_p = avcodec_find_decoder(codec_context_->codec_id);
    ret = avcodec_open2(codec_context_, codec_p, nullptr);
    if (ret != 0) {
        return false;
    }
    time_base_ = codec_context_->time_base;
    sws_context_ = sws_getContext(width_, height_, codec_context_->pix_fmt, width_, height_, AVPixelFormat::AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);
    if (!sws_context_)
    {
        return false;
    }
    return true;
}


bool VideoDecoder::Run()
{
    packet_ = (AVPacket*)av_malloc(sizeof(AVPacket));
    while(av_read_frame(decoder_, packet_) >= 0)
    {
        if (packet_->stream_index == AVMEDIA_TYPE_VIDEO)
        {
            if (avcodec_send_packet(codec_context_, packet_) != 0)
            {
                continue;
            }

            AVFrame* frame = av_frame_alloc();
            if (avcodec_receive_frame(codec_context_, frame) != 0)
            {
                av_frame_free(&frame);
                continue;
            }
            //解码线程
            std::shared_ptr<QImage> output = std::make_shared<QImage>(width_, height_, QImage::Format_ARGB32);
			auto func = [=](std::shared_ptr<QImage> img_ptr)
            {
                int64_t timestamp = frame->best_effort_timestamp * av_q2d(codec_context_->time_base) * 1000.0;
              
				return PostImageTask(sws_context_, frame, width_, height_, timestamp, std::move(img_ptr));
            };
            image_funcs_.push_back(ImageFunc(std::move(output),std::move(func)));
        }
    }
    av_free(packet_);
    return true;
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
        image_info = new ImageInfo(timestamp,output);
        return image_info;
    }
    return nullptr;
}

bool VideoDecoder::GetImage(ImageInfo*& image_info)
{
    //kThreadVideoRender
    ImageFunc delay_func;
    bool b_get = image_funcs_.get_front_read_write(delay_func);
    if (b_get) 
    {
        image_info = delay_func.delay_func_(delay_func.image_);
    }
	return b_get;
}

int VideoDecoder::GetFrameTime() const
{
    return frame_time_;
}
