#include "video_encoder.h"
#include "define/bytes_info.h"
#include "define/encoder_type.h"
#include "QImage"
#include "../../Thread/time_util.h"
#include "qbitmap.h"
#include "define/encoder_critical_sec.h"
#include "iostream"
extern "C" 
{
#include "libavcodec/avcodec.h"
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include "libavutil/fifo.h"
#include "libavformat/avformat.h"
#include "../decoder/AVFrameWrapper.h"
#include "av_packet_wrapper.h"
#include <libavutil/opt.h>
}
constexpr int pix_size = 1920 * 1080;
constexpr int packet_max_size = pix_size * 3 + 1;
VideoEncoder::VideoEncoder()
{
	sws_context_ = nullptr;
	b_stop_ = false;
	video_width_ = 1920;
	video_height_ = 1080;
	v_stream_ = nullptr;
	codec_context_ = nullptr;
	pic_src_format_ = AVPixelFormat::AV_PIX_FMT_BGR24;
}

VideoEncoder::~VideoEncoder()
{
}

void VideoEncoder::Init(const std::weak_ptr<EncoderCriticalSec>& info)
{
	encoder_info_ = info;
	AddVideoStream();
	if (!OpenVideo()) 
	{
		std::cout << "open video failed" << std::endl;
	}
}

void VideoEncoder::RunEncoder()
{
	while (!IsEnded()) 
	{
		std::shared_ptr<BytesInfo> info;
		bool b_get = msg_queue_.get_front_block(info);
		if (b_get) 
		{
			ParseBytesInfo(info);
		}
	}
	auto shared_ptr = encoder_info_.lock();
	if(shared_ptr) 
	{
		shared_ptr->WriteTrailer();
	}
}

void VideoEncoder::PostImage(std::shared_ptr<BytesInfo>&& ptr)
{
	msg_queue_.push_back_non_block(ptr);
}

void VideoEncoder::Stop()
{
	b_stop_ = true;
	msg_queue_.release();
}

bool VideoEncoder::IsEnded()
{
	return b_stop_;
}

void VideoEncoder::ParseBytesInfo(const std::shared_ptr<BytesInfo>& bytes_info)
{
	std::shared_ptr<AVFrameWrapper> frame = CreateFrame(pic_src_format_,video_width_,video_height_,(uint8_t*)bytes_info->real_bytes_);
	auto dst_frame = CreateFrame(AVPixelFormat::AV_PIX_FMT_YUV420P, video_width_, video_height_,nullptr);
	dst_frame->Frame()->pts = 0;
	AVPacketWrapper av_packet;
	av_packet.Init();
	if (NeedConvert()) 
	{
		if (!sws_context_) 
		{
			sws_context_ = sws_getContext(video_width_,video_height_, pic_src_format_,
				video_width_, video_height_,
				AVPixelFormat::AV_PIX_FMT_YUV420P,
				SWS_BICUBLIN, NULL, NULL, NULL);
		}
		sws_scale(sws_context_, frame->Frame()->data, frame->Frame()->linesize,
			0,video_height_, dst_frame->Frame()->data,dst_frame->Frame()->linesize);
	}
	dst_frame->Frame()->best_effort_timestamp = bytes_info->GetFrameTime(v_stream_->time_base);
	dst_frame->Frame()->pts = bytes_info->GetFrameTime(v_stream_->time_base);
	dst_frame->Frame()->pkt_dts = bytes_info->GetFrameTime(v_stream_->time_base);
	//// Encode frame to packet.
	int res = avcodec_send_frame(codec_context_, dst_frame->Frame());
	if(res != 0)
	{
		char error_buf[100];
		av_make_error_string(error_buf,100,res);
		std::cout << error_buf << std::endl;
		return;
	}
	res = avcodec_receive_packet(codec_context_, av_packet.Get());
	if (res != 0) 
	{
		return;
	}
	
	if (codec_context_->coded_frame->pts != AV_NOPTS_VALUE)
	{
		av_packet.Get()->pts = AV_NOPTS_VALUE;
	}
	if (codec_context_->coded_frame->key_frame)
	{
		av_packet.Get()->flags |= AV_PKT_FLAG_KEY;
	}
	av_packet.Get()->stream_index = v_stream_->index;
	std::shared_ptr<EncoderCriticalSec> encoder_shared_ptr = encoder_info_.lock();
	if (!encoder_shared_ptr)
	{
		return;
	}
		
	if (!encoder_shared_ptr->WriteFrame(av_packet))
	{
		std::cout << "write frame failed" << std::endl;
		return;
	}
}

std::shared_ptr<AVFrameWrapper> VideoEncoder::CreateFrame(const AVPixelFormat& pix_fmt, int width, int height,uint8_t* src_ptr)
{
	auto picture = std::make_shared<AVFrameWrapper>();
	
	int size = 0;

	size = avpicture_get_size(pix_fmt, width, height);
	
	if(!src_ptr)
	{
		uint8_t* picture_buf = NULL;
		picture_buf = (uint8_t*)av_malloc(size);
		av_image_fill_arrays(picture->Frame()->data,picture->Frame()->linesize,picture_buf,pix_fmt, width, height,1);
		picture->SetFreeType(FreeDataType::FREE_DATA_AV);
	}
	else 
	{
		av_image_fill_arrays(picture->Frame()->data, picture->Frame()->linesize, src_ptr, pix_fmt, width, height, 1);
		picture->SetFreeType(FreeDataType::FREE_DATA_RAW);
	}
	picture->Frame()->width = video_width_;
	picture->Frame()->height = video_height_;
	picture->Frame()->format = pix_fmt;
	return picture;
}

bool VideoEncoder::NeedConvert()
{
	return pic_src_format_ != AV_PIX_FMT_YUV420P;
}

void VideoEncoder::AddVideoStream()
{
	std::shared_ptr<EncoderCriticalSec> encoder_shared_ptr = encoder_info_.lock();
	if (!encoder_shared_ptr)
	{
		return ;
	}

	v_stream_ = encoder_shared_ptr->CreateNewStream();
	if (!v_stream_)
	{
		printf("Cannot add new vidoe stream\n");
		return ;
	}
	codec_context_ = v_stream_->codec;
	codec_context_->codec_id = (AVCodecID)encoder_shared_ptr->GetVideoCodecId();
	codec_context_->codec_type = AVMEDIA_TYPE_VIDEO;
	codec_context_->frame_number = 0;
	codec_context_->bit_rate = 4000000;
	codec_context_->width = video_width_;
	codec_context_->height = video_height_;
	codec_context_->time_base.den = 25;
	codec_context_->time_base.num = 1;
	codec_context_->gop_size = 12;
	codec_context_->qmin = 10;
	codec_context_->qmax = 51;
	codec_context_->max_qdiff = 4;
	codec_context_->me_range = 16;
	codec_context_->compression_level = 0.6;
	av_opt_set(codec_context_->priv_data, "tune", "zerolatency", 0);


	codec_context_->pix_fmt = AV_PIX_FMT_YUV420P;
	if (codec_context_->codec_id == AV_CODEC_ID_MPEG2VIDEO)
	{
		// Just for testing, we also add B frames 
		codec_context_->max_b_frames = 2;
	}
	if (codec_context_->codec_id == AV_CODEC_ID_MPEG1VIDEO)
	{
		/* Needed to avoid using macroblocks in which some coeffs overflow.
		This does not happen with normal video, it just happens here as
		the motion of the chroma plane does not match the luma plane. */
		codec_context_->mb_decision = 2;
	}

	// Some formats want stream headers to be separate.
	codec_context_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
}

bool VideoEncoder::OpenVideo()
{
	AVCodec* codec = avcodec_find_encoder(codec_context_->codec_id);
	if (!codec)
	{
		printf("Cannot found video codec\n");
		return false;
	}
	int res = avcodec_open2(codec_context_, codec, NULL);
	if (res < 0)
	{
		printf("Cannot open video codec\n");
		return false;
	}
	return true;

}
