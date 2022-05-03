#include "audio_io_device.h"
#include <iostream>
#include "../Thread/time_util.h"
#include "../view_callback/view_callback.h"
const int DURATION = 26;
AudioIoDevice::AudioIoDevice()
{
	current_read_timestamp_ = 0;
}

AudioIoDevice::~AudioIoDevice()
{
}

qint64 AudioIoDevice::readData(char* data, qint64 maxlen)
{
	auto res = bytes_list_.GetBytes(maxlen, data, current_read_timestamp_);
	ViewCallback::GetInstance()->NotifyTimeCallback(current_read_timestamp_);
	return res;
}

qint64 AudioIoDevice::writeData(const char* data, qint64 len)
{
    return 0;
}

void AudioIoDevice::Write(QByteArray& bytes, int64_t timestamp)
{
	bytes_list_.InsertBytes(bytes,timestamp);
}

int64_t AudioIoDevice::GetCurrentTimeStamp() const
{
	return current_read_timestamp_ + DURATION;
}
