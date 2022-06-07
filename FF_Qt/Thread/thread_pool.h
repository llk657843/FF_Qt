#pragma once
#include <atomic>
#include <qnamespace.h>

#include "thread_name_define.h"
#include "vector"
#include <thread>
#include "pending_task.h"
#include <unordered_map>
#include "thread_safe_priority_task_queue.h"
#include "mutex"
#include "../base_util/weak_callback.h"
#include "../base_util/singleton.h"
/*线程池，可将任务分发给指定线程*/
class QThread;
class ThreadPool : public SupportWeakCallback
{
public:
	SINGLETON_DEFINE(ThreadPool);
	ThreadPool();
	~ThreadPool();

	void Post2Task(ThreadName thread_name, const ThreadTask& f);

	template<class _Rep, class _Period>
	void Post2DelayedTask(ThreadName thread_name, const ThreadTask& f, const std::chrono::duration<_Rep, _Period>& time_span);

	template<class _Rep, class _Period>
	void Post2RepeatedTask(ThreadName thread_name, const WeakFunction<ThreadTask>& task, const std::chrono::duration<_Rep, _Period>& time_span);

	void StopAll();

private:
	void EventLoop(int name);
	void InitAll();
	bool GetThreadName(Qt::HANDLE, int& thread_name);
	Qt::HANDLE GetThreadId(int);

	template<class _Rep, class _Period>
	void PushTask(ThreadName thread_name, const ThreadTask& f, const std::chrono::duration<_Rep, _Period>& time_span);

	template<class _Rep, class _Period>
	void PrivateRepeatTask(ThreadName thread_id, const WeakFunction<ThreadTask>& task, const std::chrono::duration<_Rep, _Period>& delay);

private:
	std::atomic_bool b_done_;
	ThreadSafePriorityQueue* work_queue_;
	std::unordered_map<int, Qt::HANDLE> thread_name_to_id_;
	std::unordered_map<Qt::HANDLE, int> thread_id_to_name_;
	std::vector<QThread*> threads_;
	std::mutex thread_map_mutex_;
	
};

template<class _Rep, class _Period>
void ThreadPool::Post2RepeatedTask(ThreadName thread_name, const WeakFunction<ThreadTask>& task, const std::chrono::duration<_Rep, _Period>& time_span)
{
	auto new_task = ToWeakCallback([=]() {
		PrivateRepeatTask(thread_name, task, time_span);
	});
	Post2DelayedTask(thread_name, new_task, time_span);
}

template<class _Rep, class _Period>
void ThreadPool::PrivateRepeatTask(ThreadName thread_id, const WeakFunction<ThreadTask>& task, const std::chrono::duration<_Rep, _Period>& delay)
{
	if (task.Expired()) {
		return;
	}
	task();
	if (task.Expired()) {
		return;
	}
	Post2RepeatedTask(thread_id, task, delay);
}

template<class _Rep, class _Period>
void ThreadPool::PushTask(ThreadName thread_name, const ThreadTask& f, const std::chrono::duration<_Rep, _Period>& time_span)
{
	bool b_push_success = false;
	if (thread_name < ThreadCnt)
	{
		auto id = GetThreadId(thread_name);
		work_queue_[thread_name].push(ThreadTaskInfo(id, f, time_span));
		b_push_success = true;
	}
	//如果push到指定线程失败，则立刻执行该任务
	if (!b_push_success)
	{
		f();
	}
}

template<class _Rep, class _Period>
void ThreadPool::Post2DelayedTask(ThreadName thread_name, const ThreadTask& f, const std::chrono::duration<_Rep, _Period>& time_span)
{
	PushTask(thread_name, f, time_span);
}

