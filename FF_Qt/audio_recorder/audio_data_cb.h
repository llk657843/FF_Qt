#pragma once
#include "functional"
#include "qbytearray.h"
#include "../base_util/singleton.h"
using RecordBufferCallback = std::function<void(const QByteArray&,int64_t timestamp_ms)>;
class AudioDataCallback 
{
public:
	SINGLETON_DEFINE(AudioDataCallback)
	AudioDataCallback();
	~AudioDataCallback();
	void RegRecordBufferCallback(RecordBufferCallback);
	void NotifyBufferCallback(char* bytes,int byte_size);

private:
	RecordBufferCallback record_buffer_cb_;
};