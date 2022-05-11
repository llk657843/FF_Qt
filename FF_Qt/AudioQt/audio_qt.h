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
using PlayStartCallback = std::function<void()>;
class AudioPlayerCore : public QObject,public SupportWeakCallback
{
	Q_OBJECT
public:
	AudioPlayerCore();
	~AudioPlayerCore();
	bool Init(const std::string& path);
	void Play();
	bool IsRunning();
	int64_t GetCurrentTimestamp();
	void Pause();
	void Resume();
	bool IsPaused();
	void Clear();
	void Seek(int64_t,SeekResCallback);

signals:
	void SignalStart();

private slots:
	void SlotStart();

private:
	void SlotStateChange(QAudio::State);
	void RegCallback();
	void Close();
	void WriteByteArray(const QByteArray&, int64_t timestamp);
	void SeekContinue();

private:
	QAudioOutput* output_;
	AudioIoDevice* io_;
	int sample_rate_;
	int64_t start_time_;
	int64_t end_time_;
	bool b_stop_;
	AudioDecoder audio_decoder_;
	PlayStartCallback play_start_callback_;
	bool b_audio_seek_;
	int64_t seek_time_;
	std::atomic_bool b_start_;
	SeekResCallback seek_res_callback_;
};