#pragma once
#include <qobject.h>
#include "../Thread/threadsafe_queue.h"
#include "../Audio/char_queue.h"
class AudioIoDevice;
class QIODevice;
class QAudioOutput;
class AudioPlayerCore : public QObject
{
	Q_OBJECT
public:
	AudioPlayerCore();
	~AudioPlayerCore();
	void Play();
	void PlayFile();

signals:
	void SignalStart();

private slots:
	void SlotStart();

private:
	void InitAudioFormat();
	void WriteThread();

private:
	QAudioOutput* output_;
	AudioIoDevice* io_;
	//QIODevice* io_;
	thread_safe_queue<MyString*> char_queue_;
	QByteArray bytes_;
};