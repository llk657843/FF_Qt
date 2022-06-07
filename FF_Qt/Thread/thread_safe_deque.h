#pragma once
#include "shared_mutex"
#include "queue"
#include "mutex"
#include "condition_variable"
#include "atomic"
/*
* push back non block£¬get front block
*/
template<typename T>
class thread_safe_deque 
{
public:
	thread_safe_deque() 
	{
		b_end_ = false;
	};
	~thread_safe_deque() 
	{
		release();
	}

	void push_back_non_block(T single) 
	{
		std::unique_lock<std::mutex> lock(mutex_);
		internal_queue_.push(single);
		try_wake_up(false);
	}

	void release() 
	{
		std::unique_lock<std::mutex> lock(mutex_);
		b_end_ = true;
		while (!internal_queue_.empty())
		{
			internal_queue_.pop();
		}
		try_wake_up(true);
	}

	void end_wakeup()
	{
		std::unique_lock<std::mutex> lock(mutex_);
		b_end_ = true;
		condition_variable_.notify_one();
	}

	bool get_front(T& object)
	{
		if(b_end_)
		{
			return false;
		}
		std::unique_lock<std::mutex> lock(mutex_);
		if (!internal_queue_.empty())
		{
			object = internal_queue_.front();
			internal_queue_.pop();
			return true;
		}
		return false;
	}

	bool get_front(T& object,bool b_wait)
	{
		if (b_wait)
		{
			std::unique_lock<std::mutex> lock(mutex_);
			while (is_empty_no_lock() && !b_end_)
			{	
				condition_variable_.wait(lock);
			}
			if (b_end_)
			{
				return false;
			}
			if (!internal_queue_.empty())
			{
				object = internal_queue_.front();
				internal_queue_.pop();
				return true;
			}
			return false;
		}
		else
		{
			std::unique_lock<std::mutex> lock(mutex_);
			if (!is_empty_no_lock())
			{
				object = internal_queue_.front();
				internal_queue_.pop();
				return true;
			}
			return false;
		}
	}
	bool is_empty_lock()
	{
		std::unique_lock<std::mutex> lock(mutex_);
		return internal_queue_.empty();
	}
private:
	void try_wake_up(bool b_force)
	{
		condition_variable_.notify_all();
	}

	bool is_empty_no_lock()
	{
		return internal_queue_.empty();
	}


private:
	mutable std::mutex mutex_;
	std::queue<T> internal_queue_;
	std::condition_variable_any condition_variable_;
	std::atomic_bool b_end_;
};