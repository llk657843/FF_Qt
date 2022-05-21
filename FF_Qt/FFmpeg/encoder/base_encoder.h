#pragma once
#include "../../Thread/thread_safe_deque.h"
#include "memory"
#include "define/bytes_info.h"
class AVCodecContext;
class AVFormatContext;
class AVStream;
class AVOutputFormat;
class BaseEncoder 
{
public:
	BaseEncoder();
	virtual ~BaseEncoder();

protected:
	bool PrepareEncode(const std::string& file_name);
	AVStream* AddVideoStream();
	AVStream* AddAudioStream();
	bool OpenVideoFormat();
	bool OpenAudioFormat();

protected:
	thread_safe_deque<std::shared_ptr<BytesInfo>> msg_queue_;
	AVCodecContext* codec_context_;
	int video_index_;
	AVFormatContext* format_context_;
	AVOutputFormat* output_format_;
	int video_width_;
	int video_height_;
	AVStream* v_stream_;
	uint8_t* video_encode_buffer_;
};