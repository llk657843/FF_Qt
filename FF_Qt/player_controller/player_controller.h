#pragma once
#include <functional>
#include <memory>
#include <qaudio.h>

#include "high_ratio_time_thread.h"
#include "../base_util/singleton.h"
#include "QObject"
#include "../base_util/weak_callback.h"	
#include "../FFmpeg/ffmpeg_controller.h"
#include "QPointer"
#include "QTimer"
#include "QThread"
class VideoDecoder;
class AudioDecoder;
class AudioPlayerCore;
class PlayerController:public QObject,public SupportWeakCallback
{
	Q_OBJECT
public:
	PlayerController();
	~PlayerController();
	SINGLETON_DEFINE(PlayerController);
	void InitCallbacks();
	bool Start();
	bool Open();
	bool IsRunning();
	void Pause();
	void Resume();
	void SeekTime(int64_t seek_time);

signals:
	void SignalStartLoop();
	void SignalStopLoop();

private slots:
	void SlotStartLoop();
	void SlotStopLoop();
	void SlotMediaTimeout();

private:
	std::shared_ptr<std::function<void(QAudio::State)>> audio_state_cb_;
	WeakCallbackFlag weak_flag_;
	std::condition_variable_any cv_pause_;
	std::mutex pause_mutex_;
	std::atomic_bool bool_flag_;
	HighRatioTimeThread time_thread_;
	VideoDecoder* video_decoder_;
	AudioPlayerCore* audio_core_;
	std::string path_;
};
