#pragma once
#include <queue>
#include <mutex>
#include "pending_task.h"
/*
	线程池任务队列
	每个线程都有自己的任务队列
	需要保证线程安全，因为是由其他线程分发过来任务。自己线程完成该任务
*/
class ThreadSafePriorityQueue 
{
public:
	//************************************
	// Method:    get_top
	// Access:    public 
	// Returns:   bool
	// Qualifier: 阻塞型任务，随时可能阻塞，需要notify唤醒
	//************************************
	bool get_top(ThreadTaskInfo&);
	//************************************
	// Method:    wait_for_work
	// FullName:  ThreadSafePriorityQueue::wait_for_work
	// Access:    public 
	// Returns:   void
	// Qualifier: 闲置当前线程
	//************************************
	void wait_for_work();

	//************************************
	// Method:    release_all
	// FullName:  ThreadSafePriorityQueue::wake_up_all
	// Access:    public 
	// Returns:   void
	// Qualifier: 清空任务队列以后，唤醒所有线程
	//************************************
	void release_all();

	//************************************
	// Method:    push
	// FullName:  ThreadSafePriorityQueue::push
	// Access:    public 
	// Returns:   void
	// Qualifier: 每次push任务时，会根据任务的执行时间排序。push任务必定会唤醒线程。
	//************************************
	void push(const ThreadTaskInfo&);

private:
	size_t size();
	bool is_empty();
	
	void pop();
	ThreadTaskInfo top();

private:
	std::priority_queue<ThreadTaskInfo> task_queue_;
	std::mutex mutex_;
	std::condition_variable cv_;
};