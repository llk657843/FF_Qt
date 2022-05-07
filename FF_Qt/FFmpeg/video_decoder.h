#pragma once
extern "C"
{
#include <libavutil/rational.h>
}
#include "base_decoder.h"
#include "image_func_packet.h"
#include "../Thread/threadsafe_queue.h"

class AVCodecContext;
class AVFormatContext;
class SwsContext;
class AVFrame;
class AVPacket;
class VideoDecoder : public BaseDecoder
{
public:
	VideoDecoder();
	~VideoDecoder();
	bool Init(const std::string& path) override;
	bool Run() override;
	bool GetImage(ImageInfo*& image_info);
	int GetFrameTime() const;

private:
	ImageInfo* PostImageTask(SwsContext* sws_context, AVFrame* frame, int width, int height, int64_t timestamp, std::shared_ptr<QImage> output);

private:
	AVCodecContext* codec_context_;
	int video_stream_id_;
	int frame_time_;
	AVPacket* packet_;
	int width_;
	int height_;
	thread_safe_queue<ImageFunc> image_funcs_;
	AVRational time_base_;
	SwsContext* sws_context_;
};