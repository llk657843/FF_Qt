#include "view_callback.h"

ViewCallback::ViewCallback()
{

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
