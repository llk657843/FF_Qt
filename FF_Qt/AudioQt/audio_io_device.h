#pragma once
#include <mutex>
#include <QIODevice>
#include <shared_mutex>
#include "../Audio/bytes_list.h"
class AudioIoDevice : public QIODevice
{
public:
	AudioIoDevice();
	~AudioIoDevice();

	virtual qint64 readData(char* data, qint64 maxlen) override;
	virtual qint64 writeData(const char* data, qint64 len) override;

	void Write(const QByteArray& bytes,int64_t timestamp);
	int64_t GetCurrentTimeStamp() const;
	void Clear();

private:
	ThreadSafeBytesList bytes_list_;
	std::atomic_int64_t current_read_timestamp_;
};
