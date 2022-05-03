#pragma once
#include <functional>
#include <memory>
#include <qaudio.h>
#include "../base_util/singleton.h"
#include "QObject"
#include "../base_util/weak_callback.h"	
#include "../FFmpeg/ffmpeg_controller.h"
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

signals:
	void SignalStartLoop();
	void SignalStopLoop();

private slots:
	void SlotStartLoop();
	void SlotStopLoop();

private:
	std::unique_ptr<FFMpegController> ffmpeg_control_;
	std::shared_ptr<std::function<void(QAudio::State)>> audio_state_cb_;
	WeakCallbackFlag weak_flag_;
	std::condition_variable_any cv_pause_;
	std::mutex pause_mutex_;
	std::atomic_bool bool_flag_;
};
