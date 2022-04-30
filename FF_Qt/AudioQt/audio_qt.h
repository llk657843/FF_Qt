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
	void SetSamplerate(int sample_rate);
	void Play();
	void WriteByteArray(QByteArray&,int64_t timestamp);

signals:
	void SignalStart();

private slots:
	void SlotStart();

private:
	void InitAudioFormat();

private:
	QAudioOutput* output_;
	AudioIoDevice* io_;
	int sample_rate_;
};