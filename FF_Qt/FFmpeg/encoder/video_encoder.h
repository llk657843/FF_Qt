#pragma once
#include <cstdint>
#include "../../Thread/thread_safe_deque.h"
#include "memory"
#include "define/bytes_info.h"
#include "../../player_controller/define/video_encoder_param.h"
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
	void Init(const std::weak_ptr<EncoderCriticalSec>& info, const VideoEncoderParam& encode_param);
	void RunEncoder();
	void PostImage(std::shared_ptr<BytesInfo>&&);
	void Stop();

private:
	bool IsEnded();
	void ParseBytesInfo(const std::shared_ptr<BytesInfo>& bytes_info);
	std::shared_ptr<AVFrameWrapper> CreateFrame(const AVPixelFormat& pix_fmt, int width, int height,uint8_t* src);
	bool NeedConvert();
	void AddVideoStream(const VideoEncoderParam&);
	bool OpenVideo();
	bool SendFrame(std::shared_ptr<AVFrameWrapper> frame_wrapper);

private:
	int frame_size_;
	SwsContext* sws_context_;
	bool b_stop_;
	AVPixelFormat pic_src_format_;
	std::weak_ptr<EncoderCriticalSec> encoder_info_;
	int video_width_;
	thread_safe_deque<std::shared_ptr<BytesInfo>> msg_queue_;
	int video_height_;
	int src_width_;
	int src_height_;
	AVStream* v_stream_;
	AVCodecContext* codec_context_;
	int64_t frame_cnt_;
	int64_t packet_cnt_;
	bool b_write_success_;
};