#include "player_controller.h"

#include <iostream>
#include <qaudio.h>
#include <windows.h>
#include "../FFmpeg/video_decoder.h"
#include "../view_callback/view_callback.h"
#include "../Thread/thread_pool_entrance.h"
#include "../time_strategy/time_base_define.h"
#include "../base_util/guard_ptr.h"
#include "../AudioQt/audio_qt.h"
#include "../image_info/image_info.h"
PlayerController::PlayerController()
{
	path_ = "F:/周杰伦-一路向北-(国语)[Chedvd.com].avi";
	pause_flag_ = false;
	connect(this, SIGNAL(SignalStartLoop()), this, SLOT(SlotStartLoop()));
	connect(this, SIGNAL(SignalStopLoop()), this, SLOT(SlotStopLoop()));

	InitCallbacks();
}

PlayerController::~PlayerController()
{
	pause_flag_ = false;
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
	audio_core_->Play();
	auto video_task = ToWeakCallback([=]()
	{
		video_decoder_->Run();
	});

	qtbase::Post2Task(kThreadVideoDecoder, video_task);
	return true;
}

bool PlayerController::Open()
{
	video_decoder_ = new VideoDecoder;
	InitAudioCore();
	bool b_open = video_decoder_->Init(path_);
	if(!b_open)
	{
		return false;
	}

	b_open = audio_core_->Init(path_);
	if (!b_open)
	{
		return false;
	}
	return true;
}

bool PlayerController::IsRunning()
{
	return false;
}

void PlayerController::Pause()
{
	pause_flag_ = true;
}

void PlayerController::Resume()
{
	if (pause_flag_)
	{
		pause_flag_ = false;
		cv_pause_.notify_one();
	}
}

void PlayerController::SeekTime(int64_t seek_time)
{
	//lock start
	//lock end
}

void PlayerController::SlotStartLoop()
{
	auto time_out_cb = ToWeakCallback([=]()
		{
			SlotMediaTimeout();
		});

	video_render_thread_.RegTimeoutCallback(time_out_cb);
	video_render_thread_.Run();
}

void PlayerController::SlotStopLoop()
{
	weak_flag_.Cancel();
}

void PlayerController::SlotMediaTimeout()
{
	ImageInfo* image_info = nullptr;
	if (ParseImageInfo(image_info) != false)
	{
		if (image_info && image_info->delay_time_ms_ > 0)
		{
			video_render_thread_.SetInterval(image_info->delay_time_ms_);
		}
		
		ViewCallback::GetInstance()->NotifyImageInfoCallback(image_info);
	}
	if (pause_flag_)
	{
		std::unique_lock<std::mutex> lock(pause_mutex_);
		cv_pause_.wait(lock);
	}
}

void PlayerController::InitAudioCore()
{
	audio_core_ = new AudioPlayerCore;
}

bool PlayerController::ParseImageInfo(ImageInfo*& image_info)
{
	if (!video_decoder_)
	{
		return false;
	}
	bool b_get = video_decoder_->GetImage(image_info);
	int normal_wait_time = video_decoder_->GetFrameTime();
	if (!b_get)
	{
		return false;
	}
	int64_t audio_timestamp = 0;
	audio_timestamp = audio_core_->GetCurrentTimestamp();
	if(audio_timestamp == 0)
	{
		image_info->delay_time_ms_ = normal_wait_time;
	}
	else if (image_info && image_info->timestamp_ > audio_timestamp)
	{
		//视频比音频快，则让视频渲染等一会儿再渲染
		int64_t sleep_time = image_info->timestamp_ - audio_timestamp;
		sleep_time = sleep_time > MAX_ADJUST_TIME ? MAX_ADJUST_TIME + normal_wait_time : sleep_time + normal_wait_time;
		if (sleep_time > 0)
		{
			image_info->delay_time_ms_ = sleep_time;
		}
	}
	else if (image_info&& image_info->timestamp_ < audio_timestamp)
	{
		//视频比音频慢，则让视频渲染加快
		int sub_time = audio_timestamp - image_info->timestamp_;
		sub_time = sub_time > MAX_ADJUST_TIME ? MAX_ADJUST_TIME : sub_time;
		normal_wait_time -= sub_time;
		if (normal_wait_time > 0)
		{
			image_info->delay_time_ms_ = normal_wait_time;
		}
	}
	else if (image_info && image_info->timestamp_ <= audio_timestamp)
	{
		image_info->delay_time_ms_ = normal_wait_time;
	}
	else
	{
		return false;
	}
	return true;
}
