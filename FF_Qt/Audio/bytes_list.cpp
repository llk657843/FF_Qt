#include "bytes_list.h"
#include "../Thread/time_util.h"
ThreadSafeBytesList::ThreadSafeBytesList()
{
}

ThreadSafeBytesList::~ThreadSafeBytesList()
{
	std::lock_guard<std::mutex> lock(mutex_);
	byte_list_.clear();
}

void ThreadSafeBytesList::InsertBytes(QByteArray& bytes, int64_t timestamp)
{
	std::lock_guard<std::mutex> lock(mutex_);
	byte_list_.emplace_back(SingleByteArray(bytes,timestamp));
}

qint64 ThreadSafeBytesList::GetBytes(qint64 max_byte_size, char* res_bytes, std::atomic_int64_t& timestamp)
{
	std::lock_guard<std::mutex> lock(mutex_);
	qint64 index = 0;
	QByteArray cpy;
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
	if(cpy.size() != 0)
	{
		memcpy(res_bytes, cpy.data(), size_t(cpy.size()));
	}
	return index;
}

void ThreadSafeBytesList::Clear()
{
	std::lock_guard<std::mutex> lock(mutex_);
	byte_list_.clear();
}
