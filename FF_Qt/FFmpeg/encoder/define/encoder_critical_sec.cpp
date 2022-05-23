#include "encoder_critical_sec.h"
extern "C"
{
#include <libavutil/opt.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include "libavcodec/avcodec.h"
}
EncoderCriticalSec::EncoderCriticalSec()
{
	format_context_ = nullptr;
}

EncoderCriticalSec::~EncoderCriticalSec()
{
}

bool EncoderCriticalSec::InitFormatContext(const std::string& file_path)
{
	av_register_all();
	AVOutputFormat* output_format = av_guess_format(NULL, file_path.c_str(), NULL);
	if (!output_format)
	{
		return false;
	}
	format_context_ = avformat_alloc_context();
	if (!format_context_)
	{
		return false;
	}
	format_context_->oformat = output_format;
	memcpy(format_context_->filename, file_path.c_str(), file_path.size());
	av_dump_format(format_context_, 0, file_path.c_str(), 1);
	if (avio_open(&format_context_->pb, file_path.c_str(), AVIO_FLAG_WRITE) < 0)
	{
		return false;
		printf("Cannot open file\n");
	}
	if (avformat_write_header(format_context_, NULL) < 0)
	{
		return false;
	}
	return true;
}

AVStream* EncoderCriticalSec::CreateNewStream()
{
	std::lock_guard<std::mutex> lock(format_ctx_mtx_);
	return avformat_new_stream(format_context_, NULL);
}
