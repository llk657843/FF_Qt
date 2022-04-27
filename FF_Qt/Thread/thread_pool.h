#pragma once
#include <atomic>
#include "thread_name_define.h"
#include "vector"
#include <thread>
#include "pending_task.h"
#include <unordered_map>
#include "thread_safe_priority_task_queue.h"
#include "mutex"
#include "time_util.h"
#include "../base_util/weak_callback.h"
#include "../base_util/singleton.h"
/*线程池，可将任务分发给指定线程*/
class ThreadPool : public SupportWeakCallback
{
public:
	SINGLETON_DEFINE(ThreadPool);
	explicit ThreadPool();
	~ThreadPool();

	void Post2Task(ThreadName thread_name, const ThreadTask& f);

	template<class _Rep, class _Period>
	void Post2DelayedTask(ThreadName thread_name, const ThreadTask& f, const std::chrono::duration<_Rep, _Period>& time_span);

	template<class _Rep, class _Period>
	void Post2RepeatedTask(ThreadName thread_name, const WeakFunction<ThreadTask>& task, const std::chrono::duration<_Rep, _Period>& time_span);

	void StopAll();

private:
	void EventLoop();
	void InitAll();
	bool GetThreadName(std::thread::id, int& thread_name);
	std::thread::id GetThreadId(int);

	template<class _Rep, class _Period>
	void PushTask(ThreadName thread_name, const ThreadTask& f, const std::chrono::duration<_Rep, _Period>& time_span);
	//************************************
	// Method:    IsThreadInitDone
	// FullName:  ThreadPool::CheckThreadInitDone
	// Access:    private 
	// Returns:   bool
	// Qualifier: 线程创建完成时，线程无法得知自己对应的映射值。需要主线程通知。此时线程不处理任务。但是任务可以正常放进来。
	//			  线程会在知道自己映射后开始处理任务
	//************************************
	bool IsThreadInitDone();

	template<class _Rep, class _Period>
	void PrivateRepeatTask(ThreadName thread_id, const WeakFunction<ThreadTask>& task, const std::chrono::duration<_Rep, _Period>& delay);

private:
	std::atomic_bool b_done_;
	ThreadSafePriorityQueue* work_queue_;
	std::vector<std::thread> threads_;
	std::unordered_map<int, std::thread::id> thread_name_to_id_;
	std::unordered_map<std::thread::id, int> thread_id_to_name_;
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

