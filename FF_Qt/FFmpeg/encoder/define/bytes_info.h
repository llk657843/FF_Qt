#pragma once
#include "encoder_type.h"
#include "windows.h"
#include <cstdint>
#include <type_traits>
class BytesInfo
{
public:
	BytesInfo(EncoderDataType type, PRGBTRIPLE&& bytes, int64_t frame_time) : bytes_(std::move(bytes)),data_type_(type),frame_time_(frame_time)
	{
		
	}

public:
	PRGBTRIPLE bytes_;
	EncoderDataType data_type_;
	int64_t frame_time_;
};