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
#include "../audio_recorder/audio_data_cb.h"
EncoderController::EncoderController()
{
	video_encoder_ = nullptr;
	audio_encoder_ = nullptr;
	start_time_ = 0;
	audio_core_.InitLoop();
}

EncoderController::~EncoderController()
{
	
}

void EncoderController::ReadyEncode()
{
	std::string path = "D:\\out.mp4";
	InitEnocderInfo(path);
	InitVideoEncoder();
	InitAudio();
	encoder_info_->OpenIo();
	encoder_info_->WriteHeader();

	InitScreenCap();
}

void EncoderController::StartCatch()
{
	auto timeout_cb = ToWeakCallback([=]() {
		int64_t begin_time = time_util::GetCurrentTimeMst();
		if (start_time_ == 0) 
		{
			start_time_ = begin_time;
		}
		auto bytes = screen_cap_->GetScreenBytes();
		video_encoder_->PostImage(std::make_shared<BytesInfo>(bytes, begin_time - start_time_));
		});
	video_capture_thread_.RegTimeoutCallback(timeout_cb);
	video_capture_thread_.InitMediaTimer();
	video_capture_thread_.SetInterval(40);
	video_capture_thread_.Run();

	
	qtbase::Post2Task(kThreadVideoEncoder, [=]() {
		video_encoder_->RunEncoder(); 
		});
	qtbase::Post2Task(kThreadAudioCapture, [=]() {
		recorder_.RecordWave();
		});
}

void EncoderController::StopCapture()
{
	video_capture_thread_.Stop();
	if (video_encoder_) 
	{
		video_encoder_->Stop();
	}
	recorder_.StopRecord();
	encoder_info_->WriteTrailer();
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
		return;
	}
	audio_encoder_ = std::make_unique<AudioEncoder>();
	audio_encoder_->Init(encoder_info_);

	auto task = [=](const QByteArray& bytes, int64_t timestamp_ms) {
		audio_encoder_->PushBytes(bytes, timestamp_ms);
	};

	AudioDataCallback::GetInstance()->RegRecordBufferCallback(task);
}

void EncoderController::InitVideoEncoder()
{
	if (!video_encoder_)
	{
		video_encoder_ = std::make_unique<VideoEncoder>();
		video_encoder_->Init(encoder_info_);
	}
}
