#pragma once
#include <functional>
#include <qaudio.h>

#include "../base_util/singleton.h"
using AudioStateCallback = std::function<void(QAudio::State)>;
//unused
using OpenDoneCallback = std::function<void()>;
class ViewCallback
{
public:
	ViewCallback();
	~ViewCallback();
	SINGLETON_DEFINE(ViewCallback);
	//�Թ۲���ģʽ������Ƶ״̬
	void RegAudioStateCallback(std::shared_ptr<AudioStateCallback>);
	void NotifyAudioStateCallback(QAudio::State);

private:
	std::list<std::weak_ptr<AudioStateCallback>> audio_start_callbacks_;
};