#include "encoder_controller.h"
#include "../FFmpeg/encoder/video_encoder.h"
#include "QPixmap"
#include "QImage"
#include "QApplication"
#include "QDesktopWidget"
#include "QScreen"
#include "../Thread/time_util.h"
#include "iostream"
#include "windows.h"
#include "QtWinExtras/QtWin"
#include "../screen_capture/win_screen_cap.h"
EncoderController::EncoderController()
{
	video_encoder_ = nullptr;
}

EncoderController::~EncoderController()
{
	
}

void EncoderController::ReadyEncode()
{
	if (!video_encoder_) 
	{
		video_encoder_ = std::make_unique<VideoEncoder>();
		video_encoder_->Init();
	}
}


void EncoderController::StartCatch()
{
	screen_cap_ = new WinScreenCap;
	screen_cap_->Init();
	auto timeout_cb = ToWeakCallback([=]() {
		int64_t begin_time = time_util::GetCurrentTimeMst();
		auto bytes = screen_cap_->GetDesktopScreen();
		video_encoder_->PostImage(bytes,begin_time);
		});
	video_render_thread_.RegTimeoutCallback(timeout_cb);
	video_render_thread_.InitMediaTimer();
	video_render_thread_.SetInterval(40);
	video_render_thread_.Run();
}
