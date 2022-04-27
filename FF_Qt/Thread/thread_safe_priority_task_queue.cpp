#include "thread_safe_priority_task_queue.h"
#include "time_util.h"
//#define QUEUE_DEBUG
bool ThreadSafePriorityQueue::get_top(ThreadTaskInfo& node)
{
	std::unique_lock<std::mutex> lock(mutex_);

	if (is_empty())
	{
		return false;
	}
	node = task_queue_.top();
	auto current_time = time_util::GetCurrentTimeMs();
	if (node.pending_abs_time_ms_ <= current_time)
	{
#ifdef QUEUE_DEBUG
		printf("%d will take this job \n", std::this_thread::get_id());
#endif
		task_queue_.pop();
		lock.unlock();
		return true;
	}
	else
	{
#ifdef QUEUE_DEBUG
		printf("%d will take this job lately until time : %lld\n", std::this_thread::get_id(), node.pending_abs_time_ms_);
#endif
		std::cv_status state = cv_.wait_until(lock,node.pending_abs_time_ms_);
#ifdef QUEUE_DEBUG
		printf("now %d waked up\n", std::this_thread::get_id());
#endif
		if (state == std::cv_status::no_timeout) 
		{	
#ifdef QUEUE_DEBUG
			printf("%d want to sleep,but someone called me\n", std::this_thread::get_id());
#endif
			lock.unlock();
			return get_top(node);
		}
		else 
		{
#ifdef QUEUE_DEBUG
			printf("%d sleep well\n", std::this_thread::get_id());
#endif
			if (!is_empty()) 
			{
				task_queue_.pop();
				return true;
			}
			else 
			{
				return false;
			}
		}
	}
}

void ThreadSafePriorityQueue::wait_for_work()
{
	std::unique_lock<std::mutex> lock(mutex_);
#ifdef QUEUE_DEBUG
	printf("no work for %d ,he will sleep ,sleep in cv : %d \n", std::this_thread::get_id(), &cv_);
#endif
	cv_.wait(lock);
#ifdef QUEUE_DEBUG
	printf("%d waked up\n", std::this_thread::get_id());
#endif
}

void ThreadSafePriorityQueue::release_all()
{
	std::unique_lock<std::mutex> lock(mutex_);
	while (!task_queue_.empty())
	{
		task_queue_.pop();
	}
	cv_.notify_all();
}

void ThreadSafePriorityQueue::push(const ThreadTaskInfo& new_node)
{
	std::unique_lock<std::mutex> lock_guard(mutex_);
	task_queue_.push(new_node);
	lock_guard.unlock();
	cv_.notify_one();
}

void ThreadSafePriorityQueue::pop() 
{
	std::unique_lock<std::mutex> unique_guard(mutex_);
	while (is_empty()) 
	{
		cv_.wait(unique_guard);
	}
	task_queue_.pop();
}

ThreadTaskInfo ThreadSafePriorityQueue::top() 
{
	std::unique_lock<std::mutex> lock_guard(mutex_);
	while (is_empty()) 
	{
		cv_.wait(lock_guard);
	}
	auto top_value = task_queue_.top();
	return top_value;
}

size_t ThreadSafePriorityQueue::size() 
{
	return task_queue_.size();
}

bool ThreadSafePriorityQueue::is_empty()
{
	return task_queue_.empty();
}