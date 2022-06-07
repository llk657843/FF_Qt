#include "chrono"
#include "ctime"
#include "time_util.h"
#include <assert.h>
#include <time.h>
#include "QString"
namespace time_util
{
	std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> GetCurrentTimeMs()
	{
		return std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
	}

	__int64 GetCurrentTimeMst()
	{
		std::uint64_t ticks = 0;
		return  std::chrono::duration_cast<std::chrono::milliseconds>(GetCurrentTimeMs().time_since_epoch()).count();
	}
}
