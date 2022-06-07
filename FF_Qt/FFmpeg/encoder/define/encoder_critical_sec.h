#pragma once
#include "string"
#include "memory"
#include "mutex"
#include "../av_packet_wrapper.h"
#include "functional"
class AVFormatContext;
class AVStream;
class AVFrameWrapper;
using StopSuccessCallback = std::function<void()>;
class EncoderCriticalSec 
{
public:
	EncoderCriticalSec();
	~EncoderCriticalSec();
	//Call once
	bool InitFormatContext(const std::string& file_path);
	void OpenIo();

	bool WriteHeader();
	AVStream* CreateNewStream();
	void WriteTrailer();
	int GetVideoCodecId() const;
	int GetAudioCodecId() const;
	bool WriteFrame(AVPacketWrapper&);
	
	void RegStopSuccessCallback(StopSuccessCallback);

private:
	bool IsAllStreamReady();

private:
	AVFormatContext* format_context_;
	std::mutex format_ctx_mtx_;
	std::string file_path_;
	int end_vote_;
	int64_t last_frame_timestamp_;
	StopSuccessCallback stop_success_cb_;
	bool b_video_stream_open_;
	bool b_audio_stream_open_;
};