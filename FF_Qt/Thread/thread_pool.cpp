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
		//һ���Ի��������߳�
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
			//�˴�һ��Ҫ��b_done���ж��ź��棬��Ϊget_top��ܾ�
			if (work_queue_[name].get_top(task_info) && !b_done_)
			{
				//�����������̻߳���Ҫ��
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
				//û�����ˣ��ҿ���ȥ��Ϣ��
				work_queue_[name].wait_for_work();
			}
			else
			{
				//���������һ�������̲߳���Ҫ���ˣ��Ҿ�ֱ�ӽ������״̬���ˡ�
				continue;
			}
		}
	}
	printf("thread is exit");
}

void ThreadPool::InitAll()
{
	work_queue_ = new ThreadSafePriorityQueue[ThreadCnt];
	//Ϊÿ���̷߳���һ���������
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
		//��ʱ�˿̣��̱߳���������ˣ��������̻߳���֪�����߳�ӳ���id��ʲô
		//����취�������߳̽���ȴ�����״̬�������߳��Ѿ�֪���߳�ӳ���ʱ��ͳһ����
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