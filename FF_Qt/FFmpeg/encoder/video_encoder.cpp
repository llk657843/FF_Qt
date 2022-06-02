#include "video_encoder.h"
#include "define/bytes_info.h"
#include "define/encoder_type.h"
#include "QImage"
#include "../../Thread/time_util.h"
#include "qbitmap.h"
#include "define/encoder_critical_sec.h"
#include "iostream"
#include "algorithm"
#include <QtWidgets/qmessagebox.h>
#include "av_packet_wrapper.h"
#include "../decoder/AVFrameWrapper.h"
extern "C" 
{
#include "libavcodec/avcodec.h"
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include "libavutil/fifo.h"
#include "libavformat/avformat.h"
#include <libavutil/opt.h>
}
#define max(a,b)            (((a) > (b)) ? (a) : (b))
VideoEncoder::VideoEncoder()
{
	sws_context_ = nullptr;
	b_stop_ = false;
	video_width_ = 1920;
	video_height_ = 1080;
	src_width_ = GetSystemMetrics(SM_CXSCREEN);
	src_height_ = GetSystemMetrics(SM_CYSCREEN);
	v_stream_ = nullptr;
	frame_cnt_ = 0;
	packet_cnt_ = 0;
	b_write_success_ = false;
	pic_src_format_ = AVPixelFormat::AV_PIX_FMT_BGR24;
}

VideoEncoder::~VideoEncoder()
{
	if (sws_context_) 
	{
		sws_freeContext(sws_context_);
		sws_context_ = nullptr;
	}
	
	if(v_stream_)
	{
		if (v_stream_->codec)
		{
			avcodec_close(v_stream_->codec);
			avcodec_free_context(&v_stream_->codec);
			v_stream_->codec = nullptr;
		}
		v_stream_ = nullptr;
	}
}

void VideoEncoder::Init(const std::weak_ptr<EncoderCriticalSec>& info, const VideoEncoderParam& encode_param)
{
	encoder_info_ = info;
	video_width_ = encode_param.video_width_;
	video_height_ = encode_param.video_height_;
	AddVideoStream(encode_param);
	if (!OpenVideo()) 
	{
		std::cout << "open video failed" << std::endl;
	}
	else 
	{
		if (!sws_context_)
		{
			sws_context_ = sws_getContext(src_width_, src_height_, pic_src_format_,
				video_width_, video_height_,
				AVPixelFormat::AV_PIX_FMT_YUV420P,
				SWS_BICUBLIN, NULL, NULL, NULL);
		}
	}
}

void VideoEncoder::RunEncoder()
{
	while (true)
	{
		std::shared_ptr<BytesInfo> info;
		bool b_get = false;
		b_get = msg_queue_.get_front(info, true);
		if (b_get)
		{
			ParseBytesInfo(info);
		}
		if (IsEnded() && msg_queue_.is_empty_lock())
		{
			break;
		}
	}
	auto shared_ptr = encoder_info_.lock();
	if (shared_ptr)
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
	msg_queue_.end_wakeup();
}

bool VideoEncoder::IsEnded()
{
	return b_stop_;
}

void VideoEncoder::ParseBytesInfo(const std::shared_ptr<BytesInfo>& bytes_info)
{
	std::shared_ptr<AVFrameWrapper> frame = CreateFrame(pic_src_format_, src_width_, src_height_, (uint8_t*)bytes_info->real_bytes_);
	auto dst_frame = CreateFrame(AVPixelFormat::AV_PIX_FMT_YUV420P, video_width_, video_height_, nullptr);
	if (NeedConvert())
	{
		sws_scale(sws_context_, frame->Frame()->data, frame->Frame()->linesize,
			0, video_height_, dst_frame->Frame()->data, dst_frame->Frame()->linesize);
	}

	if (!SendFrame(dst_frame))
	{
		return;
	}

	AVPacketWrapper av_packet;
	av_packet.Init();
	auto codec_context = v_stream_->codec;
	if (avcodec_receive_packet(codec_context, av_packet.Get()) != 0)
	{
		return;
	}
	av_packet.Get()->pts = GetPts();
	packet_cnt_++;
	if (codec_context->coded_frame->key_frame)
	{
		av_packet.Get()->flags |= AV_PKT_FLAG_KEY;
	}

	av_packet.Get()->stream_index = v_stream_->index;
	std::shared_ptr<EncoderCriticalSec> encoder_shared_ptr = encoder_info_.lock();
	if (!encoder_shared_ptr)
	{
		return;
	}

	if (encoder_shared_ptr->WriteFrame(av_packet))
	{
		b_write_success_ = true;
		return;
	}
}

std::shared_ptr<AVFrameWrapper> VideoEncoder::CreateFrame(const AVPixelFormat& pix_fmt, int width, int height,uint8_t* src_ptr)
{
	auto picture = std::make_shared<AVFrameWrapper>();
	
	int	size = avpicture_get_size(pix_fmt, width, height);
	
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

void VideoEncoder::AddVideoStream(const VideoEncoderParam& video_param)
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
	auto codec_context = v_stream_->codec;
	codec_context->codec_id = (AVCodecID)encoder_shared_ptr->GetVideoCodecId();
	codec_context->codec_type = AVMEDIA_TYPE_VIDEO;
	codec_context->frame_number = 0;
	codec_context->bit_rate = video_param.bitrate_;
	codec_context->width = video_width_;
	codec_context->height = video_height_;
	codec_context->time_base.den = video_param.fps_;
	codec_context->time_base.num = 1;
	codec_context->gop_size = 12;
	codec_context->qmin = 10;
	codec_context->qmax = 51;
	codec_context->max_qdiff = 4;
	codec_context->me_range = 16;
	codec_context->compression_level = 0.6;
	av_opt_set(codec_context->priv_data, "tune", "zerolatency", 0);


	codec_context->pix_fmt = AV_PIX_FMT_YUV420P;
	if (codec_context->codec_id == AV_CODEC_ID_MPEG2VIDEO)
	{
		// Just for testing, we also add B frames 
		codec_context->max_b_frames = 2;
	}
	if (codec_context->codec_id == AV_CODEC_ID_MPEG1VIDEO)
	{
		/* Needed to avoid using macroblocks in which some coeffs overflow.
		This does not happen with normal video, it just happens here as
		the motion of the chroma plane does not match the luma plane. */
		codec_context->mb_decision = 2;
	}

	// Some formats want stream headers to be separate.
	codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
}

bool VideoEncoder::OpenVideo()
{
	auto codec = avcodec_find_encoder_by_name("h264_nvenc");
	//AVCodec* codec = avcodec_find_encoder(codec_context_->codec_id);
	if (!codec)
	{
		printf("Cannot found video codec\n");
		return false;
	}

	int res = avcodec_open2(v_stream_->codec, codec, NULL);
	if (res < 0)
	{
		printf("Cannot open video codec\n");
		return false;
	}
	return true;
}

bool VideoEncoder::SendFrame(std::shared_ptr<AVFrameWrapper> frame_wrapper)
{
	if (!b_write_success_)
	{
		frame_wrapper->Frame()->pts = frame_cnt_++;
		frame_wrapper->Frame()->best_effort_timestamp = 0;
	}
	else
	{
		frame_wrapper->Frame()->pts = GetPts();
		frame_wrapper->Frame()->best_effort_timestamp = frame_wrapper->Frame()->pts;
	}
	AVCodecContext* ctx = v_stream_->codec;
	if (ctx)
	{
		int res = avcodec_send_frame(ctx, frame_wrapper->Frame());
		if (res != 0)
		{
			char error_buf[100];
			av_make_error_string(error_buf, 100, res);
			std::cout << error_buf << std::endl;
			return false;
		}
		return true;
	}
	return false;
}

int64_t VideoEncoder::GetPts()
{
	return  av_rescale(packet_cnt_, v_stream_->time_base.den / v_stream_->time_base.num, v_stream_->codec->time_base.den / v_stream_->codec->time_base.num);
}
