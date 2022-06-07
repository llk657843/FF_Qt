#include "bytes_list.h"
#include "../Thread/time_util.h"
ThreadSafeBytesList::ThreadSafeBytesList()
{
	b_awake_ = true;
	max_size_ = 200;
}

ThreadSafeBytesList::~ThreadSafeBytesList()
{
	std::lock_guard<std::mutex> lock(mutex_);
	byte_list_.clear();
}

void ThreadSafeBytesList::InsertBytes(const QByteArray& bytes, int64_t timestamp)
{
	//Thread decoder
	std::unique_lock<std::mutex> lock(mutex_);
	byte_list_.emplace_back(SingleByteArray(bytes,timestamp));
	if(byte_list_.size() >= max_size_)
	{
		b_awake_ = false;
		cv_.wait(lock);
		b_awake_ = true;
	}
}

qint64 ThreadSafeBytesList::GetBytes(qint64 max_byte_size, char* res_bytes, std::atomic_int64_t& timestamp)
{
	//Thread render
	std::lock_guard<std::mutex> lock(mutex_);
	qint64 index = 0;
	QByteArray cpy;
	if(byte_list_.size() < (max_size_ / 2) && !b_awake_)
	{
		cv_.notify_one();
	}

	while (!byte_list_.empty() && max_byte_size > 0)
	{
		SingleByteArray single_array = byte_list_.front();
		timestamp = single_array.time_stamp_;
		int store_size = single_array.byte_array_.size();
		if (store_size <= max_byte_size)
		{
			cpy.append(std::move(single_array.byte_array_));
			max_byte_size -= store_size;
			index += store_size;
			byte_list_.pop_front();
		}
		else
		{
			cpy.append(single_array.byte_array_.mid(0,max_byte_size));
			QByteArray last = single_array.byte_array_.mid(max_byte_size,(store_size - max_byte_size));

			index += max_byte_size;
			max_byte_size = 0;
			byte_list_.pop_front();
			byte_list_.push_front(SingleByteArray(last,single_array.time_stamp_));
		}
	}
	int size = cpy.size();
	if(size != 0)
	{
		memcpy(res_bytes, cpy.data(), size_t(size));
	}
	return index;
}

void ThreadSafeBytesList::Clear()
{
	std::lock_guard<std::mutex> lock(mutex_);
	byte_list_.clear();
}
