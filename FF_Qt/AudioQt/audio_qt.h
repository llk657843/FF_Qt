#pragma once
#include <qaudio.h>
#include <qobject.h>
#include "../Thread/threadsafe_queue.h"
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
	int64_t GetCurrentTimestamp();
	void Pause();
	void Resume();

signals:
	void SignalStart();

private slots:
	void SlotStart();

private:
	void InitAudioFormat();
	void SlotStateChange(QAudio::State);

private:
	QAudioOutput* output_;
	AudioIoDevice* io_;
	int sample_rate_;
	int64_t start_time_;
	int64_t end_time_;
};