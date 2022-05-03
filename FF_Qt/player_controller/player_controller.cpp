#include "player_controller.h"

#include <iostream>
#include <qaudio.h>
#include "../view_callback/view_callback.h"
#include "../FFmpeg/ffmpeg_controller.h"
#include "../Thread/thread_pool_entrance.h"
#include "../time_strategy/time_base_define.h"
#include "../base_util/guard_ptr.h"
PlayerController::PlayerController()
{
	ffmpeg_control_ = std::make_unique<FFMpegController>();
	bool_flag_ = false;
	connect(this, SIGNAL(SignalStartLoop()), this, SLOT(SlotStartLoop()));
	connect(this, SIGNAL(SignalStopLoop()), this, SLOT(SlotStopLoop()));
	InitCallbacks();
}

PlayerController::~PlayerController()
{
	bool_flag_ = false;
	cv_pause_.notify_one();
}

void PlayerController::InitCallbacks()
{
	audio_state_cb_ = std::make_shared<std::function<void(QAudio::State)>>(ToWeakCallback([=](QAudio::State state)
	{
		if (state == QAudio::State::ActiveState)
		{
			emit SignalStartLoop();
		}
		else if (state == QAudio::State::IdleState)
		{
			emit SignalStopLoop();
		}
	}));

	ViewCallback::GetInstance()->RegAudioStateCallback(audio_state_cb_);
}


bool PlayerController::Start()
{
	if(!ffmpeg_control_)
	{
		return false;
	}
	ffmpeg_control_->AsyncOpen();
	return true;
}

bool PlayerController::Open()
{
	ffmpeg_control_->Init("F:/周杰伦-一路向北-(国语)[Chedvd.com].avi");
	return true;
}

bool PlayerController::IsRunning()
{
	return false;
}

void PlayerController::Pause()
{
	bool_flag_ = true;
	ffmpeg_control_->PauseAudio();
}

void PlayerController::Resume()
{
	if (bool_flag_) 
	{
		ffmpeg_control_->ResumeAudio();
		bool_flag_ = false;
		cv_pause_.notify_one();
	}
}

void PlayerController::SlotStartLoop()
{
	auto task = weak_flag_.ToWeakCallback([=]()
		{
			int64_t start_time = 0; 
			if (ffmpeg_control_)
			{
				ImageInfo* image_info = nullptr;
				start_time = time_util::GetCurrentTimeMst();
				if (ffmpeg_control_->GetImage(image_info) != false)
				{
					ViewCallback::GetInstance()->NotifyImageInfoCallback(image_info);
				}
			}
			int64_t end_time = time_util::GetCurrentTimeMst();
			std::cout << end_time - start_time << std::endl;
			std::unique_lock<std::mutex> lock(pause_mutex_);
			if (bool_flag_)
			{
				cv_pause_.wait(lock);
			}
		});
	qtbase::Post2RepeatedTask(kThreadVideoRender, task, std::chrono::milliseconds(BASE_SLEEP_TIME));
}

void PlayerController::SlotStopLoop()
{
	weak_flag_.Cancel();
}
