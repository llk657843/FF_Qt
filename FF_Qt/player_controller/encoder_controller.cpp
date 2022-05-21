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
#include "../Thread/thread_pool_entrance.h"
EncoderController::EncoderController()
{
	video_encoder_ = nullptr;
	start_time_ = 0;
}

EncoderController::~EncoderController()
{
	
}

void EncoderController::ReadyEncode()
{
	if (!video_encoder_) 
	{
		video_encoder_ = std::make_unique<VideoEncoder>();
		video_encoder_->Init("D:\\out.mp4");
	}
}


void EncoderController::StartCatch()
{
	screen_cap_ = new WinScreenCap;
	screen_cap_->Init();
	auto timeout_cb = ToWeakCallback([=]() {
		int64_t begin_time = time_util::GetCurrentTimeMst();
		if (start_time_ == 0) 
		{
			start_time_ = begin_time;
		}
		auto bytes = screen_cap_->GetScreenBytes();
		video_encoder_->PostImage(std::make_shared<BytesInfo>(bytes, begin_time - start_time_));
		/*auto bytes = screen_cap_->GetDesktopScreen();
		video_encoder_->PostImage(bytes,begin_time - start_time_);*/
		});
	video_render_thread_.RegTimeoutCallback(timeout_cb);
	video_render_thread_.InitMediaTimer();
	video_render_thread_.SetInterval(40);
	video_render_thread_.Run();

	
	qtbase::Post2Task(kThreadVideoDecoder, [=]() {
		video_encoder_->RunEncoder(); 
		});
}

void EncoderController::StopCapture()
{
	video_render_thread_.Stop();
	video_encoder_->Stop();
}
