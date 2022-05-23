#include "base_encoder.h"
#include "string"
#include <iostream>
extern "C" 
{
#include <libavutil/opt.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include "libavcodec/avcodec.h"
}
const int MAX_BUFFER = 1920 * 1080 * 3;
BaseEncoder::BaseEncoder()
{
	codec_context_ = nullptr;
	v_stream_ = nullptr;
	format_context_ = nullptr;
	output_format_ = nullptr;
}

BaseEncoder::~BaseEncoder()
{
}

bool BaseEncoder::PrepareEncode(const std::string& file_name)
{
	av_register_all();
	output_format_ = av_guess_format(NULL,file_name.c_str(),NULL);
	if (!output_format_) 
	{
		return false;
	}
	format_context_ = avformat_alloc_context();
	if (!format_context_) 
	{
		return false;
	}
	format_context_->oformat = output_format_;
	memcpy(format_context_->filename,file_name.c_str(),file_name.size());
	v_stream_ = AddVideoStream();
	av_dump_format(format_context_, 0, file_name.c_str(), 1);
	if (!OpenVideoFormat())
	{
		return false;
	}
	OpenAudioFormat();

	if (avio_open(&format_context_->pb, file_name.c_str(), AVIO_FLAG_WRITE) < 0)
	{
		return false;
		printf("Cannot open file\n");
	}
	if (avformat_write_header(format_context_, NULL) <0) 
	{
		return false;
	}
	return true;
}

AVStream* BaseEncoder::AddVideoStream()
{
	codec_context_ = nullptr;
	AVStream* v_stream = nullptr;
	v_stream = avformat_new_stream(format_context_, 0);
	if (!v_stream)
	{
		printf("Cannot add new vidoe stream\n");
		return NULL;
	}
	codec_context_ = v_stream->codec;
	codec_context_->codec_id = format_context_->oformat->video_codec;
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
	if (format_context_->oformat->flags & AVFMT_GLOBALHEADER)
	{
		codec_context_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}
	return v_stream;
}

AVStream* BaseEncoder::AddAudioStream()
{
	return nullptr;
}

bool BaseEncoder::OpenVideoFormat()
{
	AVCodec* codec = avcodec_find_encoder(codec_context_->codec_id);
	if (!codec) 
	{
		printf("Cannot found video codec\n");
		return false;
	}
	int res = avcodec_open2(codec_context_,codec,NULL);
	if (res < 0) 
	{
		printf("Cannot open video codec\n");
		return false;
	}
	video_encode_buffer_ = (uint8_t*)av_malloc(MAX_BUFFER);
	return true;
}

bool BaseEncoder::OpenAudioFormat()
{
	return false;
}
