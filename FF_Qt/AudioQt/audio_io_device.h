#pragma once
#include <mutex>
#include <QIODevice>
#include <shared_mutex>

class AudioIoDevice : public QIODevice
{
public:
	AudioIoDevice();
	~AudioIoDevice();

	virtual qint64 readData(char* data, qint64 maxlen) override;
	virtual qint64 writeData(const char* data, qint64 len) override;

	void Write(QByteArray bytes);
	int GetDataSize() const;

private:
	QByteArray m_data;
	int readed_index_;
	mutable std::shared_mutex data_mutex_;
};
