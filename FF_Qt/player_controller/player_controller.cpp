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
	weak_flag_.Cancel();
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

void PlayerController::SeekTime(int64_t seek_time)
{
	//lock start
	if (ffmpeg_control_)
	{
		Pause();
		auto start_time = time_util::GetCurrentTimeMst();
		while(!ffmpeg_control_->IsPaused())
		{
			std::this_thread::yield();
			auto end_time = time_util::GetCurrentTimeMst();
			if(end_time - start_time >= 100)
			{
				return;
			}
		}
		ffmpeg_control_->ClearCache();
		ffmpeg_control_->Seek(seek_time);
	}
	//lock end
}

void PlayerController::SlotStartLoop()
{
	auto time_out_cb = ToWeakCallback([=]()
		{
			SlotMediaTimeout();
		});

	time_thread_.RegTimeoutCallback(time_out_cb);
	time_thread_.Run();
}

void PlayerController::SlotStopLoop()
{
	weak_flag_.Cancel();
}

void PlayerController::SlotMediaTimeout()
{
	if (ffmpeg_control_)
	{
		ImageInfo* image_info = nullptr;
		if (ffmpeg_control_->GetImage(image_info) != false)
		{
			if (image_info && image_info->delay_time_ms_ > 0)
			{
				time_thread_.SetInterval(image_info->delay_time_ms_);
			}
			ViewCallback::GetInstance()->NotifyImageInfoCallback(image_info);
		}
		if (bool_flag_)
		{
			std::unique_lock<std::mutex> lock(pause_mutex_);
			cv_pause_.wait(lock);
		}
	}
}