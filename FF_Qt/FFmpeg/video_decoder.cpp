#include "video_decoder.h"
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
    height_ = 0;
    src_width_ = 0;
    packet_ = nullptr;
    image_funcs_.set_max_size(200); //��ຬ200�����棬Լ10�뻺��
}

VideoDecoder::~VideoDecoder()
{
    avformat_free_context(decoder_);
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
    //������������copy
    AVCodecParameters* codec_param = decoder_->streams[video_stream_id_]->codecpar;
    if (!codec_param)
    {
        return false;
    }
    //����������������
    codec_context_ = avcodec_alloc_context3(decoder_->video_codec);
    if (!codec_context_) {
        return false;
    }

    //���Ʊ����������������
    int ret = avcodec_parameters_to_context(codec_context_, codec_param);
    if (ret != 0)
    {
        return false;
    }
    
    width_ = codec_context_->width;
    height_ = codec_context_->height;
    src_height_ = codec_context_->height;
    src_width_ = codec_param->width;
    frame_time_ = 1000.0 / decoder_->streams[video_stream_id_]->avg_frame_rate.num;
    //�򿪱�����
    auto codec_p = avcodec_find_decoder(codec_context_->codec_id);
    ret = avcodec_open2(codec_context_, codec_p, nullptr);
    if (ret != 0) {
        return false;
    }
    time_base_ = codec_context_->time_base;
    format_ = codec_context_->pix_fmt;
    sws_context_ = sws_getContext(src_width_, src_height_, format_, width_, height_, AVPixelFormat::AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);
    if (!sws_context_)
    {
        return false;
    }
    return true;
}

bool VideoDecoder::Run()
{
    packet_ = (AVPacket*)av_malloc(sizeof(AVPacket));
    while(ReadFrame())
    {
        if (packet_->stream_index == AVMEDIA_TYPE_VIDEO)
        {
            if (SendPacket())
            {
                av_packet_unref(packet_);
                continue;
            }

            AVFrame* frame = av_frame_alloc();
            if (ReceiveFrame(frame))
            {
                av_packet_unref(packet_);
                av_frame_unref(frame);
                av_frame_free(&frame);
                continue;
            }
            //�����߳�
            int width = width_;
            int height = height_;
            QImage* image = new QImage(width, height, QImage::Format_ARGB32);
            std::shared_ptr<QImage> output(image);
			auto func = [=](std::shared_ptr<QImage> img_ptr)
            {
                AVRational time_base;
                {
                    std::lock_guard<std::mutex> lock(decode_mutex_);
                    time_base = codec_context_->time_base;
                }
				int64_t timestamp = frame->best_effort_timestamp * av_q2d(time_base) * 1000.0;
				return PostImageTask(frame, width, height, timestamp, std::move(img_ptr));
            };
            image_funcs_.push_back(ImageFunc(std::move(output),std::move(func)));
        }
        av_packet_unref(packet_);
    }
    av_packet_free(&packet_);
    avformat_free_context(decoder_);
    decoder_ = nullptr;
    return true;
}

ImageInfo* VideoDecoder::PostImageTask(AVFrame* frame, int width, int height, int64_t timestamp, std::shared_ptr<QImage> output)
{
    //kThreadVideoRender
    ImageInfo* image_info = nullptr;
    std::shared_ptr<QImage> img_ptr = std::move(output);
    if(width != width_ || height != height_)
    {
        img_ptr.reset();
        QImage* image = new QImage(width, height, QImage::Format_ARGB32);
        std::shared_ptr<QImage> new_image_ptr(image);
    	img_ptr.swap(new_image_ptr);
    }

    if (frame)
    {
        int output_line_size[4];
        av_image_fill_linesizes(output_line_size, AV_PIX_FMT_ARGB, width_);
        uint8_t* output_dst[] = 
        {
        	img_ptr->bits()
        };

        {
            std::lock_guard<std::mutex> lock(sws_mutex_);
            sws_scale(sws_context_, frame->data, frame->linesize, 0, src_height_, output_dst, output_line_size);
        }
    	av_frame_free(&frame);
        image_info = new ImageInfo(timestamp, img_ptr);
        return image_info;
    }
    return nullptr;
}

void VideoDecoder::RefreshScaleContext()
{
    std::lock_guard<std::mutex> lock(sws_mutex_);
    //�������Init֮���ظ�����Init���˴��ͻ��в�������,���������޲�������
    sws_freeContext(sws_context_);
    sws_context_ = nullptr;
    sws_context_ = sws_getContext(src_width_, src_height_, format_, width_, height_, AVPixelFormat::AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);
}

bool VideoDecoder::ReadFrame()
{
    std::lock_guard<std::mutex> lock(decode_mutex_);
    return av_read_frame(decoder_, packet_) >= 0;
}

bool VideoDecoder::SendPacket()
{
    std::lock_guard<std::mutex> lock(decode_mutex_);
    return avcodec_send_packet(codec_context_, packet_) != 0;
}

bool VideoDecoder::ReceiveFrame(AVFrame*& frame)
{
    std::lock_guard<std::mutex> lock(decode_mutex_);
    return avcodec_receive_frame(codec_context_, frame) != 0;
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

void VideoDecoder::SetImageSize(int width, int height)
{
    if(width_ != width || height_ != height)
    {
        width_ = width;
        height_ = height;
        RefreshScaleContext();
    }
}

void VideoDecoder::Seek(int64_t seek_time_ms)
{
    std::lock_guard<std::mutex> lock(decode_mutex_);
    image_funcs_.clear();
    image_funcs_.notify_one();
    auto time_base = codec_context_->time_base;
    int seek_frame = seek_time_ms / av_q2d(time_base) / 1000.0;
    av_seek_frame(decoder_, video_stream_id_, seek_frame, AVSEEK_FLAG_BACKWARD);
}
