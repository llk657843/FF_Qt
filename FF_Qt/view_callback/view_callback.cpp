#include "view_callback.h"
#include "../player_controller/player_controller.h"
ViewCallback::ViewCallback()
{
	last_cb_time_ = 0;
	image_info_callback_ = nullptr;
	time_cb_ = nullptr;
	parse_done_callback_ = nullptr;
	record_state_update_callback_ = nullptr;
	connect(this,SIGNAL(SignalImageInfo(ImageInfo*)),this,SLOT(SlotImageInfo(ImageInfo*)));
	connect(this,SIGNAL(SignalTimeUpdate(int64_t)),this,SLOT(SlotTimeUpdate(int64_t)));
}

ViewCallback::~ViewCallback()
{

}

void ViewCallback::RegAudioStateCallback(std::shared_ptr<AudioStateCallback> cb)
{
	audio_start_callbacks_.push_back(cb);
}

void ViewCallback::NotifyAudioStateCallback(QAudio::State state)
{
	for (auto it = audio_start_callbacks_.begin();it!=audio_start_callbacks_.end();) 
	{
		auto res_ptr = (*it).lock();
		if (res_ptr)
		{
			(*res_ptr)(state);
			it++;
		}
		else
		{
			it = audio_start_callbacks_.erase(it);
		}
	}
}

void ViewCallback::RegImageInfoCallback(ImageInfoCallback image_info_cb)
{
	image_info_callback_ = image_info_cb;
}

void ViewCallback::NotifyImageInfoCallback(ImageInfo* image_info)
{
	emit SignalImageInfo(image_info);
}

void ViewCallback::RegTimeCallback(TimeCallback time_cb)
{
	time_cb_ = time_cb;
}

void ViewCallback::NotifyTimeCallback(int64_t timestamp)
{
	if (last_cb_time_ != 0)
	{
		if ((int64_t)(timestamp * 0.001) == (int64_t)(last_cb_time_ * 0.001))
		{
			return;
		}
	}
	last_cb_time_ = timestamp;
	emit SignalTimeUpdate(timestamp);
}

void ViewCallback::RegParseDoneCallback(ParseDoneCallback parse_done)
{
	parse_done_callback_ = parse_done;
}

void ViewCallback::NotifyParseDone(int64_t duration)
{
	if(parse_done_callback_)
	{
		parse_done_callback_(duration);
	}
}

void ViewCallback::RegRecordStateUpdateCallback(RecordStateUpdateCallback cb)
{
	record_state_update_callback_ = cb;
}

void ViewCallback::NotifyRecordStateUpdate(int run_state)
{
	if (record_state_update_callback_)
	{
		record_state_update_callback_(run_state);
	}
}

void ViewCallback::Clear()
{
	parse_done_callback_ = nullptr;
	image_info_callback_ = nullptr;
	time_cb_ = nullptr;
}

void ViewCallback::SlotImageInfo(ImageInfo* image_info)
{
	if (image_info_callback_)
	{
		image_info_callback_(image_info);
	}
}

void ViewCallback::SlotTimeUpdate(int64_t timestamp)
{
	if (time_cb_)
	{
		time_cb_(timestamp);
	}
}