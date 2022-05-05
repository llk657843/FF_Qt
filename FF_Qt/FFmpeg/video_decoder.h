#pragma once
#include <list>
#include <vector>
#include "image_func_packet.h"
#include "../Thread/threadsafe_queue.h"
extern "C"
{
#include <libavcodec/avcodec.h>
}

class AVFormatContext;
class SwsContext;
class AVFrame;
class VideoDecoder
{
public:
	VideoDecoder();
	~VideoDecoder();
	bool Init(AVFormatContext* av_context);
	void Run();


private:
	ImageInfo* PostImageTask(SwsContext* sws_context, AVFrame* frame, int width, int height, int64_t timestamp, std::shared_ptr<QImage> output);

private:
	AVFormatContext* format_context_;
	int video_stream_id_;
	int frame_time_;
	AVPacket* packet_;
	int width_;
	int height_;
	thread_safe_queue<ImageFunc> image_funcs_;
	AVRational time_base_;
	SwsContext* sws_context_;
};