#pragma once
#include <mutex>
#include <QIODevice>

class AudioIoDevice : public QIODevice
{
public:
	AudioIoDevice();
	~AudioIoDevice();

	virtual qint64 readData(char* data, qint64 maxlen) override;
	virtual qint64 writeData(const char* data, qint64 len) override;

	void Write(QByteArray bytes);

private:
	QByteArray m_data;
	int current_len_;
	std::mutex data_mutex_;
};
