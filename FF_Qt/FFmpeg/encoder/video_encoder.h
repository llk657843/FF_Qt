#pragma once
#include <cstdint>
#include "../../Thread/thread_safe_deque.h"
#include "memory"
#include "define/bytes_info.h"
extern "C" 
{
#include "libavutil/pixfmt.h"
}
class AVPacket;
class AVFrame;
class AVFifoBuffer;
class SwsContext;
class AVFrameWrapper;
class EncoderCriticalSec;
class AVStream;
class AVCodecContext;
class VideoEncoder
{
public:
	VideoEncoder();
	~VideoEncoder();
	void Init(const std::weak_ptr<EncoderCriticalSec>& info);
	void RunEncoder();
	void PostImage(std::shared_ptr<BytesInfo>&&);
	void Stop();

private:
	bool IsEnded();
	void ParseBytesInfo(const std::shared_ptr<BytesInfo>& bytes_info);
	std::shared_ptr<AVFrameWrapper> CreateFrame(const AVPixelFormat& pix_fmt, int width, int height,uint8_t* src);
	bool NeedConvert();
	AVStream* AddVideoStream();
	bool OpenVideo();

private:
	AVPacket* av_packet_;
	int frame_size_;
	SwsContext* sws_context_;
	bool b_stop_;
	AVPixelFormat pic_src_format_;
	std::weak_ptr<EncoderCriticalSec> encoder_info_;
	int video_width_;
	thread_safe_deque<std::shared_ptr<BytesInfo>> msg_queue_;
	int video_height_;
	AVStream* v_stream_;
	AVCodecContext* codec_context_;
};