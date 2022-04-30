#pragma once
#include <list>
#include <qbytearray.h>
#include <shared_mutex>

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
	void InsertBytes(QByteArray& bytes,int64_t timestamp);
	qint64 GetBytes(qint64 max_byte_size,char* res_bytes,int64_t& timestamp);
	void Clear();

private:
	std::list<SingleByteArray> byte_list_;
	std::mutex mutex_;
};
