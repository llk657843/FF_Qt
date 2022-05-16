#pragma once
#include "base_encoder.h"
#include <cstdint>
#include "../../Thread/threadsafe_queue.h"
#include "memory"

class AVPacket;
class AVFrame;
class AVFifoBuffer;
class SwsContext;
class VideoEncoder : public BaseEncoder
{
public:
	VideoEncoder();
	~VideoEncoder();
	void Init();
	void RunEncoder();
	void PostImage(void* bytes,int64_t start_timestamp_ms);


private:
	bool IsEnded();
	void ParseBytesInfo(const std::shared_ptr<BytesInfo>& bytes_info);
	bool RGB24_TO_YUV420(unsigned char* RgbBuf, int w, int h, unsigned char* yuvBuf);

	unsigned char ClipValue(unsigned char x, unsigned char min_val, unsigned char max_val);

private:
	AVPacket* av_packet_;
	AVFrame* av_frame_;
	int frame_size_;
	unsigned char* out_buffer_yuv420_;
	SwsContext* sws_context_;
};