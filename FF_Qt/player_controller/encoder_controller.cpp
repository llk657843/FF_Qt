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
#include "../FFmpeg/encoder/audio_encoder.h"
#include "../FFmpeg/encoder/define/encoder_critical_sec.h"
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
	std::string path = "D:\\out.mp4";
	InitEnocderInfo(path);
	if (!video_encoder_) 
	{
		video_encoder_ = std::make_unique<VideoEncoder>();
		video_encoder_->Init(path);
	}
	InitAudio();
}


void EncoderController::StartCatch()
{
	//InitScreenCap();
	//
	//auto timeout_cb = ToWeakCallback([=]() {
	//	int64_t begin_time = time_util::GetCurrentTimeMst();
	//	if (start_time_ == 0) 
	//	{
	//		start_time_ = begin_time;
	//	}
	//	auto bytes = screen_cap_->GetScreenBytes();
	//	video_encoder_->PostImage(std::make_shared<BytesInfo>(bytes, begin_time - start_time_));
	//	});
	//video_capture_thread_.RegTimeoutCallback(timeout_cb);
	//video_capture_thread_.InitMediaTimer();
	//video_capture_thread_.SetInterval(40);
	//video_capture_thread_.Run();

	//
	//qtbase::Post2Task(kThreadAudioEncoder, [=]() {
	//	video_encoder_->RunEncoder(); 
	//	});
	qtbase::Post2Task(kThreadAudioEncoder, [=]() {
		recorder_.RecordWave();
		});
}

void EncoderController::StopCapture()
{
	video_capture_thread_.Stop();
	video_encoder_->Stop();
}

void EncoderController::InitEnocderInfo(const std::string& file_path)
{
	if (encoder_info_)
	{
		std::cout << "Encoder info already exist!" << std::endl;
		return;
	}
	encoder_info_ = std::make_shared<EncoderCriticalSec>();
	bool b_success = encoder_info_->InitFormatContext(file_path);
	if (!b_success) 
	{
		std::cout << "Encoder init failed" << std::endl;
	}

}

void EncoderController::InitScreenCap()
{
	if (!screen_cap_) 
	{
		screen_cap_ = std::make_unique<WinScreenCap>();
		screen_cap_->Init();
	}
}

void EncoderController::InitAudio()
{
	if (audio_encoder_) 
	{
		std::cout << "audio encoder already init." << std::endl;
	}
	audio_encoder_ = std::make_unique<AudioEncoder>();
	audio_encoder_->Init(encoder_info_);
}
