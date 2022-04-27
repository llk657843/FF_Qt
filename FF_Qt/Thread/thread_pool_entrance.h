#pragma once
#include "thread_name_define.h"
#include "pending_task.h"
#include "thread_pool.h"
#include <chrono>
#include "../base_util/weak_callback.h"
namespace qtbase 
{
	void Post2Task(ThreadName thread_name, const ThreadTask& task);

	template<class _Rep, class _Period>
	void Post2DelayedTask(ThreadName thread_name, const ThreadTask& task, const std::chrono::duration<_Rep, _Period>& time_span)
	{
		ThreadPool::GetInstance()->Post2DelayedTask(thread_name, task, time_span);
	}

	template<class _Rep, class _Period>
	void Post2RepeatedTask(ThreadName thread_name, const WeakFunction<ThreadTask>& task, const std::chrono::duration<_Rep, _Period>& time_span)
	{
		ThreadPool::GetInstance()->Post2RepeatedTask(thread_name, task, time_span);
	}
}