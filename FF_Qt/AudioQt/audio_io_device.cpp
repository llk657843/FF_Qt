#include "audio_io_device.h"

AudioIoDevice::AudioIoDevice()
{
}

AudioIoDevice::~AudioIoDevice()
{
}

qint64 AudioIoDevice::readData(char* data, qint64 maxlen)
{
    static int cnt = 0;
    std::lock_guard<std::mutex> lock(data_mutex_);
	if (m_data.size() >= maxlen)
    {
        QByteArray d = m_data.mid(0, int(maxlen));
        memcpy(data, d.data(), size_t(d.size()));
        m_data.remove(0, int(maxlen));
        current_len_ = m_data.size() - d.size();
        return d.size();
    }
    else 
    {
        QByteArray d = m_data;
        memcpy(data, d.data(), size_t(d.size()));
        m_data.clear();
        current_len_ = 0;
        return d.size();
    }
}

qint64 AudioIoDevice::writeData(const char* data, qint64 len)
{
    return 0;
}

void AudioIoDevice::Write(QByteArray bytes)
{
    std::lock_guard<std::mutex> lock(data_mutex_);
    m_data.append(std::move(bytes));
}
