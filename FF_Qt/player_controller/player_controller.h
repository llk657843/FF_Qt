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
	void SetAudioVolume(int value);
	int GetAudioVolume();
	void Stop();
	void SetPath(const std::string& path);

signals:
	void SignalStartLoop();
	void SignalStopLoop();
	void SignalAudioClose();
	void SignalVideoStop();

private slots:
	void SlotStartLoop();
	void SlotStopLoop();
	void SlotMediaTimeout();
	void SlotAudioClose();
	void SlotVideoStop();

private:
	void InitAudioCore();
	bool ParseImageInfo(ImageInfo*&);
	void InitVideoThread();

private:
	std::shared_ptr<std::function<void(QAudio::State)>> audio_state_cb_;
	WeakCallbackFlag weak_flag_;
	std::condition_variable_any cv_pause_;
	std::mutex pause_mutex_;
	std::atomic_bool pause_flag_;
	std::shared_ptr<HighRatioTimeThread> video_render_thread_;
	std::shared_ptr<VideoDecoder> video_decoder_;
	std::shared_ptr<AudioPlayerCore> audio_core_;
	std::string path_;
	std::string net_path_;
	int volume_;
};
