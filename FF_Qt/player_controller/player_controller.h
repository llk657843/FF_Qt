#pragma once
#include <functional>
#include <memory>
#include <qaudio.h>
#include "../base_util/singleton.h"
#include "QObject"
#include "../base_util/weak_callback.h"	
class FFMpegController;

class PlayerController:public QObject,public SupportWeakCallback
{
public:
	Q_OBJECT
	PlayerController();
	~PlayerController();
	SINGLETON_DEFINE(PlayerController);
	void InitCallbacks();
	bool Start();
	bool Open();
	bool IsRunning();

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
};
