#pragma once
#include <functional>

#include "queue"
#include "shared_mutex"
const int DEFAULT_MAX_SIZE = 8;
using RequirePushCallback = std::function<void()>;

template<typename T>
class thread_safe_queue
{
public:
	thread_safe_queue():max_size_(DEFAULT_MAX_SIZE), is_sleep_(false) {}
	~thread_safe_queue() {}
	//队列的上限值设定必须在队列开始push之前设定。上限值设定关系到缓冲区大小，也关系到内存大小
	void set_max_size(unsigned int max_size)
	{
		max_size_ = max_size;
	}

	bool is_full_lock() const
	{
		std::shared_lock<std::shared_mutex> lock(shared_mutex_);
		return internal_queue_.size() >= max_size_;
	}

	void push_back(T t)
	{
		std::unique_lock<std::shared_mutex> lock(shared_mutex_);
		while(is_full_unlock())
		{
			is_sleep_.store(true);
			condition_variable_.wait(lock);
		}
		is_sleep_.store(false);
		internal_queue_.push(t);
	}

	bool get_front_read_only(T& mem) const
	{
		std::shared_lock<std::shared_mutex> lock(shared_mutex_);
		if(!is_empty_unlock())
		{
			mem = internal_queue_.front();
			return true;
		}
		return false;
	}

	bool get_front_read_write(T& mem)
	{
		static int half_size = max_size_ / 2;
		std::lock_guard<std::shared_mutex> lock(shared_mutex_);
		if (!is_empty_unlock())
		{
			mem = internal_queue_.front();
			internal_queue_.pop();
			if (is_sleep_.load() == true && internal_queue_.size() <= half_size)
			{
				condition_variable_.notify_one();
			}
			return true;
		}
		return false;
	}

	void pop()
	{
		std::lock_guard<std::shared_mutex> lock(shared_mutex_);
		internal_queue_.pop();
	}

private:
	bool is_empty_unlock() const
	{
		return internal_queue_.empty();
	}

	bool is_full_unlock() const
	{
		return internal_queue_.size() >= max_size_;
	}

private:
	mutable std::shared_mutex shared_mutex_;
	std::queue<T> internal_queue_;
	std::condition_variable_any condition_variable_;
	std::atomic_bool is_sleep_;
	unsigned int max_size_;
};
