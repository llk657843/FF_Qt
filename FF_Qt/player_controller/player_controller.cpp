#include "player_controller.h"
#include <qaudio.h>
#include "../view_callback/view_callback.h"
#include "../FFmpeg/ffmpeg_controller.h"
#include "../Thread/thread_pool_entrance.h"
#include "../time_strategy/time_base_define.h"
PlayerController::PlayerController()
{
	ffmpeg_control_ = std::make_unique<FFMpegController>();
	connect(this, SIGNAL(SignalStartLoop()), this, SLOT(SlotStartLoop()));
	connect(this, SIGNAL(SignalStopLoop()), this, SLOT(SlotStopLoop()));
}

PlayerController::~PlayerController()
{
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

void PlayerController::SlotStartLoop()
{
	auto task = weak_flag_.ToWeakCallback([=]()
		{
			if (ffmpeg_control_)
			{
				ImageInfo* image_info = nullptr;
				if (ffmpeg_control_->GetImage(image_info) != false)
				{
					//emit SignalImage(image_info);
				}
			}
		});

	qtbase::Post2RepeatedTask(kThreadVideoRender, task, std::chrono::milliseconds(BASE_SLEEP_TIME));
}

void PlayerController::SlotStopLoop()
{
	weak_flag_.Cancel();
}
