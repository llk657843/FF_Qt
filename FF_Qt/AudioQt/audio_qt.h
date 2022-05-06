#pragma once
#include <qaudio.h>
#include <qobject.h>
#include "../Thread/threadsafe_queue.h"
#include "../base_util/weak_callback.h"
#include "../base_util/singleton.h"
#include "../FFmpeg/audio_decoder.h"
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
	void WriteByteArray(const QByteArray&,int64_t timestamp);
	int64_t GetCurrentTimestamp();
	void Pause();
	void Resume();
	bool IsPaused();
	void Clear();

signals:
	void SignalStart();

private slots:
	void SlotStart();

private:
	void SlotStateChange(QAudio::State);
	void StartLoopReadBytes();
	void RegCallback();

private:
	QAudioOutput* output_;
	AudioIoDevice* io_;
	int sample_rate_;
	int64_t start_time_;
	int64_t end_time_;
	bool b_stop_;
	AudioDecoder audio_decoder_;
};