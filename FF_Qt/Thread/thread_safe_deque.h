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
		b_sleep_ = false;
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

	bool get_front_block(T& object)
	{
		std::unique_lock<std::mutex> lock(mutex_);
		while (is_empty_no_lock() && !b_end_)
		{
			b_sleep_ = true;
			condition_variable_.wait(lock);
			b_sleep_ = false;
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

private:
	void try_wake_up(bool b_force)
	{
		if (b_sleep_ || b_force)
		{
			condition_variable_.notify_one();
		}
	}

	bool is_empty_no_lock()
	{
		return internal_queue_.empty();
	}


private:
	mutable std::mutex mutex_;
	std::queue<T> internal_queue_;
	std::condition_variable_any condition_variable_;
	std::atomic_bool b_sleep_;
	std::atomic_bool b_end_;
};