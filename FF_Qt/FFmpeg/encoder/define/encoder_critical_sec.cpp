#include "encoder_critical_sec.h"
#include "../../decoder/AVFrameWrapper.h"
#include "iostream"
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
	write_packet_vote_ = 0;
}

EncoderCriticalSec::~EncoderCriticalSec()
{
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
	std::lock_guard<std::mutex> lock(format_ctx_mtx_);
	if (end_vote_ > 0)
	{
		//vote to write, only all stream is ready to write,then write
		if(write_packet_vote_ != GetStreamIndexBinary(end_vote_ - 1))
		{
			write_packet_vote_ = write_packet_vote_ | (GetStreamIndexBinary(av_packet.Get()->stream_index));
		}
		
		if (write_packet_vote_ == GetStreamIndexBinary(end_vote_ - 1)) 
		{
			return av_interleaved_write_frame(format_context_, av_packet.Get()) == 0;
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

int EncoderCriticalSec::GetStreamIndexBinary(int index)
{
	if (index <= 31) 
	{
		return pow(2, index);
	}
	else 
	{
		std::cout << "warning,wrong stream index" << std::endl;
		return 0;
	}
	
}
