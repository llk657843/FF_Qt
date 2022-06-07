#include "thread_pool.h"
#include <qthread.h>
#include "functional"
#include "iostream"
#include "chrono"
#include <thread>
//#define THREAD_DEBUG_LOG
ThreadPool::ThreadPool()
{
	b_done_.store(false);
	InitAll();
}

ThreadPool::~ThreadPool()
{
	b_done_.store(true);
	if (work_queue_)
	{
		delete[]work_queue_;
		work_queue_ = nullptr;
	}
}

void ThreadPool::StopAll()
{
	b_done_.store(true);
	for (int i = 0; i < ThreadCnt; i++)
	{
		//һ���Ի��������߳�
		work_queue_[i].release_all();
	}

	for (int i = 0; i < threads_.size(); i++)
	{
		threads_[i]->quit();
	}
	printf("all worker thread uninstalled\n");
}

void ThreadPool::EventLoop(int name)
{
	Qt::HANDLE id = QThread::currentThreadId();
	thread_id_to_name_[id] = name;
	thread_name_to_id_[name] = id;
	while (b_done_.load() == false)
	{
		if (work_queue_)
		{
			ThreadTaskInfo task_info;
			//�˴�һ��Ҫ��b_done���ж��ź��棬��Ϊget_top��ܾ�
			if (work_queue_[name].get_top(task_info) && b_done_.load() == false)
			{
				//�����������̻߳���Ҫ��
				task_info.RunTask();
#ifdef THREAD_DEBUG_LOG
				printf("now work is done by %d\n", id);
#endif		
			}
			else if (b_done_.load() == false && work_queue_)
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
	printf("thread is exit\n");
}

void ThreadPool::InitAll()
{
	work_queue_ = new ThreadSafePriorityQueue[ThreadCnt];
	//Ϊÿ���̷߳���һ���������

	auto task = ToWeakCallback([=](int name)
	{
		EventLoop(name);
	});

	for (int i = 0; i < ThreadCnt; i++)
	{
		QThread* my_thread = QThread::create(task,i);
		threads_.push_back(my_thread);
		my_thread->start();
	}
	printf("all thread init complete\n");
	work_queue_[0].release_all();
}

bool ThreadPool::GetThreadName(Qt::HANDLE id, int& thread_name)
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

Qt::HANDLE ThreadPool::GetThreadId(int thread_name)
{
	std::lock_guard<std::mutex> lock_guard(thread_map_mutex_);
	auto it = thread_name_to_id_.find(thread_name);
	if (it != thread_name_to_id_.end())
	{
		return it->second;
	}
	return Qt::HANDLE();
}

void ThreadPool::Post2Task(ThreadName thread_name, const ThreadTask& f)
{
	auto id = GetThreadId(thread_name);
	if (id == QThread::currentThreadId()) 
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