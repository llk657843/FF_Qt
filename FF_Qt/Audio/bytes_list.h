#pragma once
#include <list>
#include <qbytearray.h>
#include <shared_mutex>
#include "condition_variable"
struct SingleByteArray
{
	SingleByteArray(QByteArray bytes,int64_t timestamp) : byte_array_(std::move(bytes)),time_stamp_(timestamp)
	{
	}
	int64_t time_stamp_;
	QByteArray byte_array_;
};


class ThreadSafeBytesList
{
public:
	ThreadSafeBytesList();
	~ThreadSafeBytesList();
	void InsertBytes(const QByteArray& bytes,int64_t timestamp);
	qint64 GetBytes(qint64 max_byte_size,char* res_bytes,std::atomic_int64_t& timestamp);
	void Clear();

private:
	std::list<SingleByteArray> byte_list_;
	std::mutex mutex_;
	std::condition_variable_any cv_;
	std::atomic_bool b_awake_;
	int max_size_;
};
