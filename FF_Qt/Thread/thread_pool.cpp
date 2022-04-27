#include "thread_pool.h"
#include "qlogging.h"
#include "iostream"
#include "chrono"
#include <thread>
#include "memory"
#include "../base_util/weak_callback.h"
//#define THREAD_DEBUG_LOG
ThreadPool::ThreadPool()
{
	b_done_ = false;
	InitAll();
}

ThreadPool::~ThreadPool()
{
	b_done_ = true;
	if (work_queue_)
	{
		delete[]work_queue_;
		work_queue_ = nullptr;
	}
}

void ThreadPool::StopAll()
{
	b_done_ = true;
	for (int i = 0; i < ThreadCnt; i++)
	{
		//一次性唤醒所有线程
		work_queue_[i].release_all();
	}

	for (int i = 0; i < threads_.size(); i++)
	{
		if (threads_[i].joinable())
		{
			threads_[i].join();
		}
	}
	printf("all worker thread uninstalled\n");
}

void ThreadPool::EventLoop()
{
	auto id = std::this_thread::get_id();
	while (!b_done_)
	{
		if (!IsThreadInitDone())
		{
			continue;
		}
		int name = 0;
		GetThreadName(id, name);
#ifdef THREAD_DEBUG_LOG
		printf("worker is ready to work %d ,my thread name : %d\n", id, name);
#endif
		if (work_queue_)
		{
			ThreadTaskInfo task_info;
			//此处一定要将b_done的判定放后面，因为get_top会很久
			if (work_queue_[name].get_top(task_info) && !b_done_)
			{
				//有任务，且主线程还需要我
				task_info.RunTask();
#ifdef THREAD_DEBUG_LOG
				printf("now work is done by %d\n", id);
#endif		
			}
			else if (!b_done_ && work_queue_)
			{
#ifdef THREAD_DEBUG_LOG
				printf("no work for %d thread,waiting for work\n", name);
#endif	
				//没任务了，我可以去休息了
				work_queue_[name].wait_for_work();
			}
			else
			{
				//其他情况，一般是主线程不需要我了，我就直接进入毁灭状态好了。
				continue;
			}
		}
	}
	printf("thread is exit");
}

void ThreadPool::InitAll()
{
	work_queue_ = new ThreadSafePriorityQueue[ThreadCnt];
	//为每个线程分配一个任务队列
	for (int i = 0; i < ThreadCnt; i++)
	{
		threads_.push_back(std::thread(&ThreadPool::EventLoop, this));
		auto id = threads_[i].get_id();
		thread_name_to_id_[i] = id;
		thread_id_to_name_[id] = i;
	}
	printf("all thread init complete\n");
	work_queue_[0].release_all();
}

bool ThreadPool::GetThreadName(std::thread::id id, int& thread_name)
{
	std::lock_guard<std::mutex> lock_guard(thread_map_mutex_);
	auto it = thread_id_to_name_.find(id);
	if (it != thread_id_to_name_.end())
	{
		thread_name = it->second;
		return true;
	}
	return false;
}

std::thread::id ThreadPool::GetThreadId(int thread_name)
{
	std::lock_guard<std::mutex> lock_guard(thread_map_mutex_);
	auto it = thread_name_to_id_.find(thread_name);
	if (it != thread_name_to_id_.end())
	{
		return it->second;
	}
	return std::thread::id();
}

bool ThreadPool::IsThreadInitDone()
{
	int name = 0;
	bool b_get_thread_name = GetThreadName(std::this_thread::get_id(), name);
	if (!b_get_thread_name && work_queue_)
	{
		//此时此刻，线程被创建完毕了，但是主线程还不知道该线程映射的id是什么
		//解决办法是先让线程进入等待工作状态，等主线程已经知道线程映射的时候统一唤醒
		work_queue_[0].wait_for_work();
		return false;
	}
	return true;
}

void ThreadPool::Post2Task(ThreadName thread_name, const ThreadTask& f)
{
	auto id = GetThreadId(thread_name);
	if (id == std::this_thread::get_id()) 
	{
		if (f) 
		{
			f();
		}
	}
	else 
	{
		PushTask(thread_name, f, std::chrono::milliseconds(0));
	}
}