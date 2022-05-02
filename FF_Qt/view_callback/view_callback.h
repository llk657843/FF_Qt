#pragma once
#include <functional>

#include "../base_util/singleton.h"
using AudioStartCallback = std::function<void()>;
class ViewCallback
{
public:
	ViewCallback();
	~ViewCallback();
	SINGLETON_DEFINE(ViewCallback);
	void RegAudioStartCallback(AudioStartCallback);
	void NotifyAudioStartCallback();

private:
	AudioStartCallback audio_start_callback_;
};