#include "native_audio_controller.h"
#include "../audio_recorder/audio_filter.h"
#include "../audio_recorder/win_audio_recorder.h"
#include "../Thread/thread_pool_entrance.h"
#include "iostream"
#include "../Thread/high_ratio_time_thread.h"
const int RECORD_STREAM_NUM = 2;
NativeAudioController::NativeAudioController()
{
	frame_data_cb_ = nullptr;
	stop_record_cb_ = nullptr;
	close_stream_cnt_ = 0;
}

NativeAudioController::~NativeAudioController()
{
}

void NativeAudioController::StartRun()
{
	InitRecorder();
	InitFilter();
	recorder_->RecordWave(-1);
	mic_recorder_->RecordWave(1);
	StartGetBufferLoop();
}

void NativeAudioController::RegDataCallback(AVFrameDataCallback cb)
{
	frame_data_cb_ = cb;
}

void NativeAudioController::RegStopRecordCallback(StopRecordCallback cb)
{
	stop_record_cb_ = cb;
}

void NativeAudioController::StopRunAsync()
{
	RecordClose();
	time_thread_->Stop();
}

void NativeAudioController::InitRecorder()
{
	recorder_ = std::make_unique<WinAudioRecorder>();
	
	auto recorder_data_cb = ([=](char* bytes, int byte_size) {
		AddStream(0,bytes,byte_size);
		});

	auto record_close_cb = ([=]() {
		close_stream_cnt_++;
		if (close_stream_cnt_ == RECORD_STREAM_NUM) 
		{
			audio_filter_.reset();
			if (stop_record_cb_)
			{
				stop_record_cb_();
			}
		}

		});

	recorder_->RegDataCallback(recorder_data_cb);
	recorder_->RegRecordCloseCallback(record_close_cb);

	auto mic_data_cb = ([=](char* bytes, int byte_size) {
		AddStream(1,bytes,byte_size);
		});


	mic_recorder_ = std::make_unique<WinAudioRecorder>();
	mic_recorder_->RegRecordCloseCallback(record_close_cb);
	mic_recorder_->RegDataCallback(mic_data_cb);
}

void NativeAudioController::InitFilter()
{
	audio_filter_ = std::make_unique<AudioFilter>();
	audio_filter_->AddAudioInput(0, 44100, 2, 12800, AVSampleFormat::AV_SAMPLE_FMT_S16);
	audio_filter_->AddAudioInput(1, 44100, 2, 12800, AVSampleFormat::AV_SAMPLE_FMT_S16);
	audio_filter_->AddAudioOutput(44100, 2, 12800, AVSampleFormat::AV_SAMPLE_FMT_FLTP);
	audio_filter_->InitFilter();
}

void NativeAudioController::RecordClose()
{
	if (mic_recorder_) 
	{
		mic_recorder_->StopRecord();
	}
	if (recorder_) 
	{
		recorder_->StopRecord();
	}
}

void NativeAudioController::AddStream(int index,char* buffer,int buffer_size)
{
	if(audio_filter_)
	{
		audio_filter_->AddFrame(index, (uint8_t*)buffer, buffer_size);
	}
}

void NativeAudioController::StartGetBufferLoop()
{
	auto media_timer = ([=]() {
		if(audio_filter_)
		{
			int res_size = 0;
			bool b_get = true;
			do 
			{
				auto av_frame = audio_filter_->GetFrame(b_get, 8192, res_size);
				if (b_get) 
				{
					if (frame_data_cb_)
					{
						frame_data_cb_(av_frame, res_size);
					}
				}
			} while (b_get);
		}
		});
	time_thread_ = std::make_unique<HighRatioTimeThread>(false);
	time_thread_->InitMediaTimer();
	time_thread_->RegTimeoutCallback(media_timer);
	time_thread_->SetInterval(10);
	time_thread_->Run();
}
