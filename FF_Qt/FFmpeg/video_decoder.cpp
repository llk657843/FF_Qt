#include "video_decoder.h"

#include "AVFrameWrapper.h"
#include "QImage"
#include "memory"
#include "../image_info/image_info.h"
#include "../view_callback/view_callback.h"
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
    src_height_ = 0;
    width_ = 0;
    b_stop_flag_ = false;
    height_ = 0;
    src_width_ = 0;
    packet_ = nullptr;
    image_funcs_.set_max_size(200); //最多含200个缓存，约10秒缓存
}

VideoDecoder::~VideoDecoder()
{
    if (decoder_) 
    {
        avformat_free_context(decoder_);
    }
}

bool VideoDecoder::Init(const std::string& path)
{
    bool b_success = PrepareDeocde(path);
    ViewCallback::GetInstance()->NotifyParseDone(decoder_->duration);
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
    
    src_height_ = codec_context_->height;
    src_width_ = codec_context_->width;
    frame_time_ = 1000.0 / decoder_->streams[video_stream_id_]->avg_frame_rate.num;
    //打开编码器
    auto codec_p = avcodec_find_decoder(codec_context_->codec_id);
    ret = avcodec_open2(codec_context_, codec_p, nullptr);
    if (ret != 0) {
        return false;
    }
    time_base_ = codec_context_->time_base;
    format_ = codec_context_->pix_fmt;
    return true;
}

bool VideoDecoder::Run()
{
    b_running_flag_ = true;
    packet_ = (AVPacket*)av_malloc(sizeof(AVPacket));
    while(ReadFrame(packet_))
    {
        if (b_stop_flag_) 
        {
            av_packet_unref(packet_);
            break;
        }
        if (packet_->stream_index == AVMEDIA_TYPE_VIDEO)
        {
            if (SendPacket(codec_context_,packet_))
            {
                av_packet_unref(packet_);
                continue;
            }

            auto frame_ptr = std::make_shared<AVFrameWrapper>();
            if (ReceiveFrame(codec_context_,frame_ptr->frame_))
            {
                av_packet_unref(packet_);
                continue;
            }
            //解码线程
            int width = width_;
            int height = height_;
			auto func = [=](std::shared_ptr<QImage> img_ptr,const std::shared_ptr<AVFrameWrapper>& cached_frame)
            {
                //kThreadVideoRender
                AVRational time_base;
                {
                    std::lock_guard<std::mutex> lock(decode_mutex_);
                    time_base = codec_context_->time_base;
                }
				int64_t timestamp = cached_frame->frame_->best_effort_timestamp * av_q2d(time_base) * 1000.0;
				return PostImageTask(std::move(cached_frame), width, height, timestamp, std::move(img_ptr));
            };
            image_funcs_.push_back(ImageFunc(std::make_shared<QImage>(width, height, QImage::Format_ARGB32),func, frame_ptr));
        }
        av_packet_unref(packet_);
    }

    ReleaseAll();
    return true;
}

ImageInfo* VideoDecoder::PostImageTask(std::shared_ptr<AVFrameWrapper> frame, int width, int height, int64_t timestamp, std::shared_ptr<QImage> output)
{
    //kThreadVideoRender
    std::lock_guard<std::mutex> lock(sws_mutex_);
    ImageInfo* image_info = nullptr;
    std::shared_ptr<QImage> img_ptr = std::move(output);
    if(width != width_ || height != height_)
    {
        img_ptr.reset();
        std::shared_ptr<QImage> new_image_ptr = std::make_shared<QImage>(width_, height_, QImage::Format_ARGB32);
    	img_ptr.swap(new_image_ptr);
    }
    auto local_frame = frame->Frame();
    if (local_frame)
    {
        int* linesize = local_frame->linesize;
        uint8_t** data = local_frame->data;
    	int output_line_size[4];
        av_image_fill_linesizes(output_line_size, AV_PIX_FMT_ARGB, width_);
        uint8_t* output_dst[] =
        {
            img_ptr->bits()
        };
        auto srs = sws_getContext(src_width_, src_height_, format_, width_, height_, AVPixelFormat::AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);
    	sws_scale(srs, data, linesize, 0, src_height_, output_dst, output_line_size);
        image_info = new ImageInfo(timestamp, img_ptr);
        sws_freeContext(srs);
    	return image_info;
    }
    return nullptr;
}

void VideoDecoder::RefreshScaleContext(int new_width,int new_height)
{
    std::lock_guard<std::mutex> lock(sws_mutex_);
    width_ = new_width;
    height_ = new_height;
}

void VideoDecoder::ReleaseAll()
{
    std::lock_guard<std::mutex> lock(decode_mutex_);
    image_funcs_.clear();
    b_running_flag_ = false;
    av_packet_free(&packet_);
    packet_ = nullptr;
    
    avcodec_free_context(&codec_context_);
    codec_context_ = nullptr;
    
    avformat_free_context(decoder_);
    decoder_ = nullptr;
    b_stop_flag_ = false;
    image_funcs_.clear();
}

bool VideoDecoder::GetImage(ImageInfo*& image_info)
{
    //kThreadVideoRender
    ImageFunc delay_func;
    bool b_get = image_funcs_.get_front_read_write(delay_func);
    if (b_get) 
    {
        image_info = delay_func.delay_func_(delay_func.image_,delay_func.frame_);
    }
	return b_get;
}

int VideoDecoder::GetFrameTime() const
{
    return frame_time_;
}

void VideoDecoder::SetImageSize(int width, int height)
{
    if(width_ != width || height_ != height)
    {
        RefreshScaleContext(width,height);
    }
}

void VideoDecoder::Seek(int64_t seek_time_ms)
{
    std::lock_guard<std::mutex> lock(decode_mutex_);
    image_funcs_.clear();
    image_funcs_.notify_one(true);
    auto time_base = codec_context_->time_base;
    int seek_frame = seek_time_ms / av_q2d(time_base) / 1000.0;
    av_seek_frame(decoder_, video_stream_id_, seek_frame, AVSEEK_FLAG_BACKWARD);
}

void VideoDecoder::Seek(int64_t seek_frame, int audio_stream_id)
{
    std::lock_guard<std::mutex> lock(decode_mutex_);
    image_funcs_.clear();
    image_funcs_.notify_one(true);
    av_seek_frame(decoder_, audio_stream_id, seek_frame, AVSEEK_FLAG_BACKWARD) >= 0 ? true : false;
}

void VideoDecoder::AsyncStop()
{
    if (b_running_flag_) 
    {
        b_stop_flag_ = true;
        image_funcs_.notify_one(true);
    }
    else 
    {
        ReleaseAll();
    }
}