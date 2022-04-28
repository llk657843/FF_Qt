#include "audio_io_device.h"

AudioIoDevice::AudioIoDevice()
{
    readed_index_ = 0;
}

AudioIoDevice::~AudioIoDevice()
{
}

qint64 AudioIoDevice::readData(char* data, qint64 maxlen)
{
    std::shared_lock<std::shared_mutex> lock(data_mutex_);
	if (m_data.size() >= maxlen)
    {
        QByteArray d = m_data.mid(readed_index_, int(maxlen));
        memcpy(data, d.data(), size_t(d.size()));
        readed_index_ += d.size();
        return d.size();
    }
    else 
    {
        QByteArray d = m_data;
        memcpy(data, d.data(), size_t(d.size()));
        readed_index_ += d.size();
    	m_data.clear();
        return d.size();
    }
}

qint64 AudioIoDevice::writeData(const char* data, qint64 len)
{
    return 0;
}

void AudioIoDevice::Write(QByteArray bytes)
{
    std::lock_guard<std::shared_mutex> lock(data_mutex_);
    m_data.append(std::move(bytes));
}

int AudioIoDevice::GetDataSize() const
{
    std::shared_lock<std::shared_mutex> lock(data_mutex_);
    return m_data.size();
}
