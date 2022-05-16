#include "base_encoder.h"
#include "string"
extern "C" 
{
#include <libavutil/opt.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

BaseEncoder::BaseEncoder()
{
}

BaseEncoder::~BaseEncoder()
{
}

bool BaseEncoder::PrepareEncode()
{
	std::string file_name = "out.mp4";

	int ret = avformat_alloc_output_context2(&encoder_context_, NULL, NULL, file_name.c_str());
	if (ret < 0)
	{
		printf("Init avformat object is faild! \n");
		return false;
	}

	AVCodec* coder = avcodec_find_encoder(encoder_context_->oformat->video_codec);
	if (!coder) 
	{
		return false;
	}

	codec_context_ = avcodec_alloc_context3(coder);
	if (!codec_context_)
	{
		return false;
	}

	av_stream_ = avformat_new_stream(encoder_context_, coder);
	
	if (!av_stream_)
	{
		printf("Init avStream object is faild! \n");
		return false;
	}
	video_index_ = av_stream_->index;
	codec_context_->flags |= AV_CODEC_FLAG_QSCALE;
	codec_context_->bit_rate = 4000000;
	codec_context_->rc_min_rate = 4000000;
	codec_context_->rc_max_rate = 4000000;
	codec_context_->bit_rate_tolerance = 4000000;
	codec_context_->time_base.den = 25;
	codec_context_->time_base.num = 1;

	codec_context_->width = 1920;
	codec_context_->height = 1080;
	//pH264Encoder->pCodecCtx->frame_number = 1;
	codec_context_->gop_size = 12;
	codec_context_->max_b_frames = 0;
	codec_context_->thread_count = 4;
	codec_context_->pix_fmt = AV_PIX_FMT_YUV420P;
	codec_context_->codec_id = AV_CODEC_ID_H264;
	codec_context_->codec_type = AVMEDIA_TYPE_VIDEO;

	av_opt_set(codec_context_->priv_data, "b-pyramid", "none", 0);
	av_opt_set(codec_context_->priv_data, "preset", "superfast", 0);
	av_opt_set(codec_context_->priv_data, "tune", "zerolatency", 0);

	int res = avcodec_open2(codec_context_,coder,NULL);
	if (res != 0) 
	{
		return false;
	}

	return true;
}
