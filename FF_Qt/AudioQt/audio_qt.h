#pragma once
#include <qobject.h>
#include "../Thread/threadsafe_queue.h"
#include "../Audio/char_queue.h"
#include "../base_util/weak_callback.h"
class AudioIoDevice;
class QIODevice;
class QAudioOutput;
class AudioPlayerCore : public QObject,public SupportWeakCallback
{
	Q_OBJECT
public:
	AudioPlayerCore();
	~AudioPlayerCore();
	void Play();
	void PlayFile();
	void WriteByteArray(QByteArray);

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
	QByteArray bytes_;
};