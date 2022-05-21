#pragma once
#include "base_encoder.h"
#include <cstdint>
#include "../../Thread/threadsafe_queue.h"
#include "memory"
extern "C" 
{
#include "libavutil/pixfmt.h"
}
class AVPacket;
class AVFrame;
class AVFifoBuffer;
class SwsContext;
class AVFrameWrapper;
class VideoEncoder : public BaseEncoder
{
public:
	VideoEncoder();
	~VideoEncoder();
	void Init(const std::string& file_name);
	void RunEncoder();
	void PostImage(std::shared_ptr<BytesInfo>&&);
	void Stop();

private:
	bool IsEnded();
	void ParseBytesInfo(const std::shared_ptr<BytesInfo>& bytes_info);
	void ParseImageInfo(const std::shared_ptr<BytesInfo>& bytes_info);
	std::shared_ptr<AVFrameWrapper> CreateFrame(const AVPixelFormat& pix_fmt, int width, int height,uint8_t* src);
	bool NeedConvert();

private:
	AVPacket* av_packet_;
	int frame_size_;
	SwsContext* sws_context_;
	bool b_stop_;
};