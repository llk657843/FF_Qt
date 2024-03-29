#include "player_controller.h"
#include <iostream>
#include <qaudio.h>
#include <windows.h>
#include "../FFmpeg/decoder/video_decoder.h"
#include "../view_callback/view_callback.h"
#include "../Thread/thread_pool_entrance.h"
#include "../time_strategy/time_base_define.h"
#include "../base_util/guard_ptr.h"
#include "../AudioQt/audio_qt.h"
#include "../image_info/image_info.h"
PlayerController::PlayerController()
{
	path_ = "";
	volume_ = 100;
	pause_flag_ = false;
	connect(this, SIGNAL(SignalStartLoop()), this, SLOT(SlotStartLoop()));
	connect(this, SIGNAL(SignalStopLoop()), this, SLOT(SlotStopLoop()));
	connect(this,SIGNAL(SignalAudioClose()),this,SLOT(SlotAudioClose()));
	connect(this,SIGNAL(SignalVideoStop()),this,SLOT(SlotVideoStop()));
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
	if(!video_decoder_)
	{
		std::cout << "warning, decoder not init!" << std::endl;
		return false;
	}


	if (audio_core_) 
	{
		audio_core_->Play();
	}
	auto video_task = [=]()
	{
		video_decoder_->Run();
	};
	
	qtbase::Post2Task(kThreadVideoDecoder, video_task);
	return true;
}

bool PlayerController::Open(int win_width,int win_height)
{
	if(video_decoder_ || audio_core_)
	{
		std::cout << "release video decoder first" << std::endl;
		return false;
	}

	auto stop_video_cb = [=]()
	{
		emit SignalVideoStop();
	};

	video_decoder_ = std::make_shared<VideoDecoder>();
	video_decoder_->RegStopSuccessCallback(stop_video_cb);
	video_decoder_->SetImageSize(win_width, win_height);
	InitAudioCore();
	InitVideoThread();
	if (!net_path_.empty()) 
	{
		path_ = net_path_;
	}
	bool b_open_video = video_decoder_->Init(path_);
	bool b_open_audio = audio_core_->Init(path_);
	if (!b_open_audio && !b_open_video)
	{
		return false;
	}
	return true;
}

bool PlayerController::IsRunning()
{
	if(!audio_core_ || audio_core_ && audio_core_->IsRunning())
	{
		return false;
	}
	return true;
}

void PlayerController::Pause()
{
	if (audio_core_) 
	{
		pause_flag_ = true;
		audio_core_->Pause();
	}
}

void PlayerController::Resume()
{
	if (pause_flag_)
	{
		pause_flag_ = false;
		cv_pause_.notify_one();
		audio_core_->Resume();
	}
}

void PlayerController::SeekTime(int64_t seek_time)
{
	auto cb = [=](int64_t seek_frame, int audio_id,bool b_success)
	{
			if (b_success && video_decoder_)
			{
				video_decoder_->Seek(seek_frame,audio_id,seek_time);
			}
	};

	if (audio_core_)
	{
		audio_core_->Seek(seek_time,cb);
	}
}

void PlayerController::SetImageSize(int width, int height)
{
	if (video_decoder_) 
	{
		video_decoder_->SetImageSize(width, height);
	}
}

void PlayerController::SetAudioVolume(int value)
{
	if (value >= 0 && value <= 100) 
	{
		volume_ = value;
		if (audio_core_)
		{
			audio_core_->SetVolume(value);
		}
	}
}

int PlayerController::GetAudioVolume()
{
	return volume_;
}

void PlayerController::Stop()
{
	if (video_render_thread_) 
	{
		video_render_thread_->Stop();
		video_render_thread_.reset();
	}
	if (video_decoder_)
	{
		video_decoder_->AsyncStop();
	}
	if (audio_core_) 
	{
		audio_core_->AsyncStop();
	}
}

void PlayerController::SetPath(const std::string& path)
{
	path_ = path;
}

void PlayerController::SlotStartLoop()
{
	if (video_render_thread_) 
	{
		video_render_thread_->Run();
	}
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
			if (video_render_thread_) 
			{
				video_render_thread_->SetInterval(image_info->delay_time_ms_);
			}
		}
		
		ViewCallback::GetInstance()->NotifyImageInfoCallback(image_info);
	}
	if (pause_flag_)
	{
		std::unique_lock<std::mutex> lock(pause_mutex_);
		cv_pause_.wait(lock);
	}
}

void PlayerController::SlotAudioClose()
{
	if (audio_core_) 
	{
		audio_core_.reset();
	}
}

void PlayerController::SlotVideoStop()
{
	if (video_decoder_) 
	{
		video_decoder_.reset();
	}
}

void PlayerController::InitAudioCore()
{
	if (audio_core_) 
	{
		return;
	}
	audio_core_ = std::make_shared<AudioPlayerCore>();
	if (volume_ > 0) 
	{
		SetAudioVolume(volume_);
	}
	else
	{
		SetAudioVolume(100);
	}
	auto close_state_cb = [=]() 
	{
		//trans to main thread
		emit SignalAudioClose();
	};

	audio_core_->RegCloseSuccessCallback(close_state_cb);
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

void PlayerController::InitVideoThread()
{
	auto time_out_cb = ToWeakCallback([=]()
		{
			SlotMediaTimeout();
		});
	video_render_thread_ = std::make_shared<HighRatioTimeThread>();
	video_render_thread_->RegTimeoutCallback(time_out_cb);
}
