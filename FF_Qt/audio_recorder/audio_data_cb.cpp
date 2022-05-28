#include "audio_data_cb.h"
#include "../Thread/thread_pool_entrance.h"
AudioDataCallback::AudioDataCallback()
{
	record_buffer_cb_ = nullptr;
}

AudioDataCallback::~AudioDataCallback()
{
}
void AudioDataCallback::RegRecordBufferCallback(RecordBufferCallback cb)
{
	record_buffer_cb_ = cb;
}

void AudioDataCallback::NotifyBufferCallback(char* bytes,int byte_size)
{
	if (record_buffer_cb_)
	{
		QByteArray cb_bytes(bytes,byte_size);
		auto task = [=]() {
			if (record_buffer_cb_)
			{
				record_buffer_cb_(cb_bytes);
			}
		};
		qtbase::Post2Task(kThreadAudioEncoder, task);
	}
}
