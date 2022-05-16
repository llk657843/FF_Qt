#pragma once
#include "iostream"
#include "queue"
#include "shared_mutex"
const int DEFAULT_MAX_SIZE = 8;
//���Ԫ����������ȡԪ�ز������������ڴ���ٶ�Զ������ȡ�ٶȵĳ���
template<typename T>
class thread_safe_queue
{
public:
	thread_safe_queue() :max_size_(DEFAULT_MAX_SIZE), is_sleep_(false), half_size_(4) {}
	~thread_safe_queue() {}
	//���е�����ֵ�趨�����ڶ��п�ʼpush֮ǰ�趨������ֵ�趨��ϵ����������С��Ҳ��ϵ���ڴ��С
	void set_max_size(unsigned int max_size)
	{
		max_size_ = max_size;
		half_size_ = max_size / 2;
	}

	bool is_full_lock() const
	{
		std::shared_lock<std::shared_mutex> lock(shared_mutex_);
		return internal_queue_.size() >= max_size_;
	}

	void push_back(T t)
	{
		std::unique_lock<std::shared_mutex> lock(shared_mutex_);
		while (is_full_unlock())
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
		if (!is_empty_unlock())
		{
			mem = internal_queue_.front();
			return true;
		}
		return false;
	}

	bool get_front_read_write(T& mem)
	{
		std::lock_guard<std::shared_mutex> lock(shared_mutex_);
		if (!is_empty_unlock())
		{
			mem = internal_queue_.front();
			internal_queue_.pop();
			notify_one(false);
			return true;
		}
		return false;
	}

	void pop()
	{
		std::lock_guard<std::shared_mutex> lock(shared_mutex_);
		internal_queue_.pop();
	}

	void clear()
	{
		std::lock_guard<std::shared_mutex> lock(shared_mutex_);
		int size = internal_queue_.size();
		while (!is_empty_unlock())
		{
			internal_queue_.pop();
		}

	}

	bool is_empty_lock() const
	{
		std::shared_lock<std::shared_mutex> lock(shared_mutex_);
		return internal_queue_.empty();
	}

	void notify_one(bool b_force)
	{
		if (b_force) 
		{
			condition_variable_.notify_one();
		}
		else if (is_sleep_.load() == true && internal_queue_.size() <= half_size_)
		{
			condition_variable_.notify_one();
		}
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
	unsigned int half_size_;
};
