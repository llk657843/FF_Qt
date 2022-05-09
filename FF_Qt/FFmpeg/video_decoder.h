#pragma once


extern "C"
{
#include <libavutil/pixfmt.h>
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
class AVFrameWrapper;
class VideoDecoder : public BaseDecoder
{
public:
	VideoDecoder();
	~VideoDecoder();
	bool Init(const std::string& path) override;
	bool Run() override;
	bool GetImage(ImageInfo*& image_info);
	int GetFrameTime() const;
	void SetImageSize(int width,int height);
	void Seek(int64_t seek_time);

private:
	ImageInfo* PostImageTask(std::shared_ptr<AVFrameWrapper> frame, int width, int height, int64_t timestamp, std::shared_ptr<QImage> output);
	void RefreshScaleContext(int new_width,int new_height);
	bool ReadFrame();
	bool SendPacket();
	bool ReceiveFrame(AVFrame*&);

private:
	AVCodecContext* codec_context_;
	int video_stream_id_;
	int frame_time_;
	AVPacket* packet_;
	int width_;
	int height_;
	int src_height_;
	int src_width_;
	AVPixelFormat format_;
	thread_safe_queue<ImageFunc> image_funcs_;
	AVRational time_base_;
	SwsContext* sws_context_;
	std::mutex sws_mutex_;

};