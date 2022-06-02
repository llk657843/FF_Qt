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
#include <QtCore/qfileinfo.h>
#include "../view_callback/view_callback.h"
#define INCLUDE_VIDEO
#define INCLUDE_AUDIO
const int pix_size = 1920 * 1080 * 3;
EncoderController::EncoderController()
{
	video_encoder_ = nullptr;
	audio_encoder_ = nullptr;
	record_state_ = RECORD_STATE_NONE;
	connect(this,&EncoderController::SignalStopSuccess,this,&EncoderController::SlotStopSuccess);
}

EncoderController::~EncoderController()
{
	
}

void EncoderController::ReadyEncode()
{
	InitEnocderInfo(file_path_.toStdString());
#ifdef INCLUDE_VIDEO
	InitVideoEncoder();
	InitScreenCap();
#endif // INCLUDE_VIDEO

#ifdef INCLUDE_AUDIO
	InitAudio();
#endif // INCLUDE_AUDIO
	encoder_info_->OpenIo();
	encoder_info_->WriteHeader();
}

void EncoderController::StartCatch()
{
	record_state_ = RecordState::RECORD_STATE_RUNNING;
	ViewCallback::GetInstance()->NotifyRecordStateUpdate(true);
#ifdef INCLUDE_VIDEO
	auto timeout_cb = ToWeakCallback([=]() {
		CaptureImage();
		});
	video_capture_thread_ = std::make_unique<HighRatioTimeThread>();
	video_capture_thread_->RegTimeoutCallback(timeout_cb);
	video_capture_thread_->InitMediaTimer();
	if (video_param_.fps_ != 0)
	{
		video_capture_thread_->SetInterval((1000 / video_param_.fps_));
	}
	else
	{
		video_capture_thread_->SetInterval(40);
	}
	video_capture_thread_->Run();
	qtbase::Post2Task(kThreadVideoEncoder, [=]() {
		video_encoder_->RunEncoder();
		});

#endif // INCLUDE_VIDEO



#ifdef INCLUDE_AUDIO
	qtbase::Post2Task(kThreadAudioCapture, [=]() {
		if (recorder_) {
			recorder_->RecordWave();
		}
	});
#endif // INCLUDE_AUDIO


}

void EncoderController::StopCapture()
{
	record_state_ = RecordState::RECORD_STATE_NONE;
	if (video_capture_thread_) 
	{
		video_capture_thread_->Stop();
	}
	if (video_encoder_) 
	{
		video_encoder_->Stop();
	}
#ifdef INCLUDE_AUDIO
	if (recorder_)
	{
		recorder_->StopRecord();
	}
	if (audio_encoder_) 
	{
		audio_encoder_->Stop();
	}
#endif // INCLUDE_AUDIO
	ViewCallback::GetInstance()->NotifyRecordStateUpdate(false);
}

void EncoderController::SetBitrate(int bitrate)
{
	video_param_.bitrate_ = bitrate;
}

void EncoderController::SetFramerate(int frame_rate)
{
	video_param_.fps_ = frame_rate;
}

void EncoderController::SetPixRate(int width, int height)
{
	video_param_.video_width_ = width;
	video_param_.video_height_ = height;
}

bool EncoderController::SetFilePath(QString file_path)
{
	QFileInfo file_info(file_path);
	if (file_info.exists()) 
	{
		file_path_ = file_path;
		return true;
	}
	return false;
}

RecordState EncoderController::GetRecordState()
{
	return record_state_;
}

QString EncoderController::GetCapturePath()
{
	return file_path_;
}

void EncoderController::StartTestMemoryLeak()
{
	InitEnocderInfo("D:\\record.mp4");
	InitVideoEncoder();
}

void EncoderController::EndTestMemoryLeak()
{
	video_encoder_.reset();
	encoder_info_.reset();
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
	auto stop_success_cb = [=]() {
		emit SignalStopSuccess();
	};

	encoder_info_->RegStopSuccessCallback(stop_success_cb);
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
	recorder_ = std::make_unique<WinAudioRecorder>();
	audio_encoder_ = std::make_unique<AudioEncoder>();
	audio_encoder_->Init(encoder_info_);

	auto task = [=](const QByteArray& bytes) {
		audio_encoder_->PushBytes(bytes);
	};

	AudioDataCallback::GetInstance()->RegRecordBufferCallback(task);
}

void EncoderController::InitVideoEncoder()
{
	if (!video_encoder_)
	{
		video_encoder_ = std::make_unique<VideoEncoder>();
		video_encoder_->Init(encoder_info_, video_param_);
	}
}

void EncoderController::CaptureImage()
{
	auto task = ToWeakCallback([=]() {
		if(!video_encoder_)
		{
			return;
		}
		auto bytes = screen_cap_->GetScreenBytes();
		if (video_encoder_) 
		{
			uint8_t* bytes_cpy = new uint8_t[pix_size + 1];
			memcpy(bytes_cpy, bytes, pix_size);
			bytes_cpy[pix_size] = '\0';
			video_encoder_->PostImage(std::make_shared<BytesInfo>(bytes_cpy));
		}
	});
	qtbase::Post2Task(kThreadVideoCapture,task);
}

void EncoderController::CleanAll()
{
	record_state_ = RecordState::RECORD_STATE_NONE;
	
	ViewCallback::GetInstance()->NotifyRecordStateUpdate(RecordState::RECORD_STATE_NONE);
	if (video_encoder_)
	{
		std::cout << "stop video" << std::endl;
		video_encoder_.reset();
	}
	if (audio_encoder_)
	{
		std::cout << "stop audio" << std::endl;
		audio_encoder_.reset();
	}
	if (encoder_info_)
	{
		std::cout << "stop encoder" << std::endl;
		encoder_info_.reset();
	}
	if (recorder_) 
	{
		std::cout << "stop recorder" << std::endl;
		recorder_.reset();
	}
	if (screen_cap_) 
	{
		std::cout << "stop screen_cap" << std::endl;
		screen_cap_.reset();
	}
	if (video_capture_thread_) 
	{
		video_capture_thread_.reset();
	}
	std::cout << "stop success" << std::endl;
}

void EncoderController::SlotStopSuccess()
{
	CleanAll();
}
