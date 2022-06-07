#pragma once
#include <functional>
#include <memory>
#include <qaudio.h>
#include "../Thread/high_ratio_time_thread.h"
#include "../base_util/singleton.h"
#include "QObject"
#include "../base_util/weak_callback.h"	
#include "QPointer"
#include "QThread"
class ImageInfo;
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
	bool Open(int win_width,int win_height);
	bool IsRunning();
	void Pause();
	void Resume();
	void SeekTime(int64_t seek_time);
	void SetImageSize(int width,int height);
	void Stop();
	void SetPath(const std::string& path);

signals:
	void SignalStartLoop();
	void SignalStopLoop();

private slots:
	void SlotStartLoop();
	void SlotStopLoop();
	void SlotMediaTimeout();

private:
	void InitAudioCore();
	bool ParseImageInfo(ImageInfo*&);

private:
	std::shared_ptr<std::function<void(QAudio::State)>> audio_state_cb_;
	WeakCallbackFlag weak_flag_;
	std::condition_variable_any cv_pause_;
	std::mutex pause_mutex_;
	std::atomic_bool pause_flag_;
	HighRatioTimeThread video_render_thread_;
	VideoDecoder* video_decoder_;
	AudioPlayerCore* audio_core_;
	std::string path_;
	std::string net_path_;
};
