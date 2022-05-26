#pragma once
#include "string"
#include "memory"
#include "mutex"
#include "../av_packet_wrapper.h"
class AVFormatContext;
class AVStream;
class AVFrameWrapper;
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


private:
	AVFormatContext* format_context_;
	std::mutex format_ctx_mtx_;
	std::string file_path_;
	int end_vote_;
};