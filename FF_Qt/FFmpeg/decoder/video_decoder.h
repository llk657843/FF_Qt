#pragma once


extern "C"
{
#include <libavutil/pixfmt.h>
#include <libavutil/rational.h>
}
#include "base_decoder.h"
#include "image_func_packet.h"
#include "../../Thread/threadsafe_queue.h"

class AVCodecContext;
class AVFormatContext;
class SwsContext;
class AVFrame;
class AVPacket;
class AVFrameWrapper;
using StopSuccessCallback = std::function<void()>;
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
	void Seek(int64_t seek_frame,int audio_stream_id,int64_t seek_time);
	void AsyncStop();
	void RegStopSuccessCallback(StopSuccessCallback);

private:
	ImageInfo* PostImageTask(std::shared_ptr<AVFrameWrapper> frame, int width, int height, int64_t timestamp, std::shared_ptr<QImage> output);
	void RefreshScaleContext(int new_width,int new_height);

	void ReleaseAll();

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
	std::mutex sws_mutex_;
	std::atomic_bool b_stop_flag_;
	std::atomic_bool b_running_flag_;
	bool b_init_success_;
	StopSuccessCallback stop_success_callback_;
	bool b_seek_flag_;
	int64_t last_seek_time_;
};