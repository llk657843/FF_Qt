#pragma once
#include "../../Thread/thread_safe_deque.h"
#include "memory"
#include "define/bytes_info.h"
class AVCodecContext;
class AVFormatContext;
class AVStream;
class BaseEncoder 
{
public:
	BaseEncoder();
	virtual ~BaseEncoder();

protected:
	bool PrepareEncode();

protected:
	thread_safe_deque<std::shared_ptr<BytesInfo>> msg_queue_;
	AVCodecContext* codec_context_;
	int video_index_;
	AVFormatContext* encoder_context_;

private:
	AVStream* av_stream_;
};