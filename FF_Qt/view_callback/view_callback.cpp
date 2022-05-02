#include "view_callback.h"

ViewCallback::ViewCallback()
{
	audio_start_callback_ = nullptr;
}

ViewCallback::~ViewCallback()
{
	audio_start_callback_ = nullptr;
}

void ViewCallback::RegAudioStartCallback(AudioStartCallback cb)
{
	audio_start_callback_ = cb;
}

void ViewCallback::NotifyAudioStartCallback()
{
	if(audio_start_callback_)
	{
		audio_start_callback_();
	}
}
