#pragma once
#include "encoder_type.h"
#include "windows.h"
#include <cstdint>
#include <type_traits>
extern "C" 
{
#include "libavutil/rational.h"
}
class BytesInfo
{
public:
	BytesInfo(unsigned char* bytes, int64_t frame_time)
	{
		real_bytes_ = bytes;
		frame_time_ = frame_time;
	}
	~BytesInfo()
	{

	}
	int64_t GetFrameTime(AVRational time_base)
	{
		return  frame_time_ / av_q2d(time_base) / 1000.0;
	}


public:
	unsigned char* real_bytes_;

private:
	EncoderDataType data_type_;
	int64_t frame_time_;
	int64_t best_effort_video_;
};