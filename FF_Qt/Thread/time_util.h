#pragma once
#include <string>
namespace time_util 
{
	std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> GetCurrentTimeMs();	//获取ms级时间戳
	__int64 GetCurrentTimeMst();	//获取ms级时间戳
}