#pragma once
#include "../base_util/singleton.h"
#include "memory"
#include "../Thread/high_ratio_time_thread.h"
#include "../base_util/weak_callback.h"	
class VideoEncoder;
class WinScreenCap;
class EncoderController : public SupportWeakCallback
{
public:
	SINGLETON_DEFINE(EncoderController);
	EncoderController();
	~EncoderController();

	void ReadyEncode();
	void StartCatch();
	void StopCapture();

private:
	std::unique_ptr<VideoEncoder> video_encoder_;
	WinScreenCap* screen_cap_;
	HighRatioTimeThread video_render_thread_;
	int64_t start_time_;
};