#pragma once
#include <functional>
#include <thread>
#include "time_util.h"
#include <memory>
#include <qnamespace.h>
using ThreadTask =std::function<void()>;
class ThreadTaskInfo
{
public:
	~ThreadTaskInfo();
	Qt::HANDLE working_thread_id_;	//需要执行的线程的id
	std::shared_ptr<ThreadTask> task_;
	std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> pending_abs_time_ms_;	//绝对时间
	ThreadTaskInfo()
	{
		
	};

	template<class _Rep, class _Period>
	ThreadTaskInfo(Qt::HANDLE id, const ThreadTask& task, const std::chrono::duration<_Rep, _Period>& delay_times)
	{
		working_thread_id_ = id;
		task_ = std::make_shared<ThreadTask>(task);
		auto mil_sec = std::chrono::duration_cast<std::chrono::milliseconds>(delay_times);
		pending_abs_time_ms_ = time_util::GetCurrentTimeMs() + std::chrono::milliseconds(mil_sec);
	};
	bool operator<(const ThreadTaskInfo& other) const 
	{
		return this->pending_abs_time_ms_ > other.pending_abs_time_ms_;
	}
	void RunTask();
};