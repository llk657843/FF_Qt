#pragma once
#include "encoder_type.h"
#include "windows.h"
#include <cstdint>
#include <type_traits>
#include "iostream"
#include "memory"
extern "C" 
{
#include "libavutil/rational.h"
}
class BytesInfo
{
public:
	BytesInfo(unsigned char* bytes)
	{
		real_bytes_ = bytes;
	}
	~BytesInfo()
	{
		delete[] real_bytes_;
	}

public:
	unsigned char* real_bytes_;
};