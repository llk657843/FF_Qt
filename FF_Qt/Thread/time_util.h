#pragma once
#include <string>
namespace time_util 
{
	std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> GetCurrentTimeMs();	//��ȡms��ʱ���
	__int64 GetCurrentTimeMst();	//��ȡms��ʱ���
}