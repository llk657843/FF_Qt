#pragma once
#include <queue>
#include <mutex>
#include "pending_task.h"
/*
	�̳߳��������
	ÿ���̶߳����Լ����������
	��Ҫ��֤�̰߳�ȫ����Ϊ���������̷ַ߳����������Լ��߳���ɸ�����
*/
class ThreadSafePriorityQueue 
{
public:
	//************************************
	// Method:    get_top
	// Access:    public 
	// Returns:   bool
	// Qualifier: ������������ʱ������������Ҫnotify����
	//************************************
	bool get_top(ThreadTaskInfo&);
	//************************************
	// Method:    wait_for_work
	// FullName:  ThreadSafePriorityQueue::wait_for_work
	// Access:    public 
	// Returns:   void
	// Qualifier: ���õ�ǰ�߳�
	//************************************
	void wait_for_work();

	//************************************
	// Method:    release_all
	// FullName:  ThreadSafePriorityQueue::wake_up_all
	// Access:    public 
	// Returns:   void
	// Qualifier: �����������Ժ󣬻��������߳�
	//************************************
	void release_all();

	//************************************
	// Method:    push
	// FullName:  ThreadSafePriorityQueue::push
	// Access:    public 
	// Returns:   void
	// Qualifier: ÿ��push����ʱ������������ִ��ʱ������push����ض��ỽ���̡߳�
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