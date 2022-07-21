#include "encoder_critical_sec.h"
#include "../../decoder/AVFrameWrapper.h"
#include "iostream"
#include "../../Thread/time_util.h"
#include "../../../view_callback/view_callback.h"
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
	end_vote_ = 0;
	stop_success_cb_ = nullptr;
	b_video_stream_open_ = false;
	b_audio_stream_open_ = false;
}

EncoderCriticalSec::~EncoderCriticalSec()
{
	if (format_context_ && format_context_->pb)
	{
		avio_close(format_context_->pb);
		format_context_->pb = NULL;
	}
	if (format_context_) 
	{
		avformat_free_context(format_context_);
		format_context_ = nullptr;
	}
}

bool EncoderCriticalSec::InitFormatContext(const std::string& file_path)
{
	av_register_all();
	file_path_ = file_path;
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

	return true;
}

void EncoderCriticalSec::OpenIo()
{
	if (avio_open(&format_context_->pb, file_path_.c_str(), AVIO_FLAG_WRITE) < 0)
	{
		printf("Cannot open file\n");
	}
}

bool EncoderCriticalSec::WriteHeader()
{
	std::lock_guard<std::mutex> lock(format_ctx_mtx_);
	if (avformat_write_header(format_context_, NULL) < 0)
	{
		return false;
	}
	return true;
}

AVStream* EncoderCriticalSec::CreateNewStream()
{
	std::lock_guard<std::mutex> lock(format_ctx_mtx_);
	end_vote_++;
	return avformat_new_stream(format_context_, NULL);
}

void EncoderCriticalSec::WriteTrailer()
{
	std::lock_guard<std::mutex> lock(format_ctx_mtx_);
	if (--end_vote_ == 0)
	{
		av_write_trailer(format_context_);
		
	}
	if (stop_success_cb_)
	{
		stop_success_cb_();
	}
}

int EncoderCriticalSec::GetVideoCodecId() const
{
	return format_context_->oformat->video_codec;
}

int EncoderCriticalSec::GetAudioCodecId() const
{
	return format_context_->oformat->audio_codec;
}

bool EncoderCriticalSec::WriteFrame(AVPacketWrapper& av_packet)
{
	int64_t start_time = time_util::GetCurrentTimeMst();
	std::lock_guard<std::mutex> lock(format_ctx_mtx_);
	if (end_vote_ > 0)
	{
		//vote to write, only all stream is ready to write,then write
		if(av_packet.Get()->stream_index == 0)
		{
			b_video_stream_open_ = true;
		}
		else if(av_packet.Get()->stream_index == 1)
		{
			b_audio_stream_open_ = true;
		}

		if(av_packet.Get()->pts  == AV_NOPTS_VALUE)
		{
			return false;
		}
		if (IsAllStreamReady()) 
		{
			int res = av_interleaved_write_frame(format_context_, av_packet.Get()) == 0;
			return res;
		}
		else 
		{
			return false;
		}
	}
	else 
	{
		return false;
	}
}

void EncoderCriticalSec::RegStopSuccessCallback(StopSuccessCallback cb)
{
	stop_success_cb_ = cb;
}

bool EncoderCriticalSec::IsAllStreamReady()
{
	if(b_video_stream_open_ && b_audio_stream_open_)
	{
		return true;
	}
	return false;
}