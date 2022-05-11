#include "audio_qt.h"

#include <iostream>
#include <QAudioOutput>
#include "QIODevice"
#include "typeinfo"
#include "../Thread/thread_pool_entrance.h"
#include "audio_io_device.h"
#include "QFile"
#include "../view_callback/view_callback.h"
#include "../player_controller/player_controller.h"
AudioPlayerCore::AudioPlayerCore()
{
	seek_res_callback_ = nullptr;
	b_audio_seek_ = false;
	b_stop_ = false;
	seek_time_ = 0;
	output_ = nullptr;
	io_ = nullptr;
	output_ = nullptr;
	b_start_ = false;
	sample_rate_ = 44100;
	play_start_callback_ = nullptr;
	connect(this,SIGNAL(SignalStart()),this,SLOT(SlotStart()));
	RegCallback();
}

AudioPlayerCore::~AudioPlayerCore()
{
}

bool AudioPlayerCore::Init(const std::string& path)
{
	bool b_init = audio_decoder_.Init(path);
	if(!b_init)
	{
		return false;
	}
	Clear();
	Close();
	sample_rate_ = audio_decoder_.GetSamplerate();
	return true;
}

void AudioPlayerCore::Play()
{
	QAudioFormat fmt;//������Ƶ�����ʽ
	fmt.setSampleRate(sample_rate_);//1�����Ƶ������
	fmt.setSampleSize(16);//���������Ĵ�С
	fmt.setChannelCount(2);//����
	fmt.setCodec("audio/pcm");//�����ʽ
	fmt.setByteOrder(QAudioFormat::LittleEndian);
	fmt.setSampleType(QAudioFormat::UnSignedInt);//������Ƶ����
	output_ = new QAudioOutput(fmt);
	io_ = new AudioIoDevice;
	io_->open(QIODevice::ReadWrite);
	connect(output_, &QAudioOutput::stateChanged, this, &AudioPlayerCore::SlotStateChange);
	auto decoder_task = ToWeakCallback([=]()
	{
		audio_decoder_.Run();
	});

	qtbase::Post2Task(kThreadAudioDecoder,decoder_task);
}

bool AudioPlayerCore::IsRunning()
{
	if(output_)
	{
		return output_->state() != QAudio::State::StoppedState;
	}
	return false;
}

void AudioPlayerCore::SlotStart()
{
	if (output_ && io_ && !b_start_)
	{
		b_start_ = true;
		output_->start(io_);//���ſ�ʼ
	}
}

void AudioPlayerCore::SlotStateChange(QAudio::State state)
{
	ViewCallback::GetInstance()->NotifyAudioStateCallback(state);
	if (state == QAudio::State::IdleState)
	{
		
	}
	else if(state == QAudio::State::ActiveState)
	{
		
	}
	else if (state == QAudio::State::StoppedState)
	{
		
	}
	else if(state == QAudio::State::SuspendedState)
	{
		if (b_audio_seek_)
		{
			SeekContinue();
		}
	}
}

void AudioPlayerCore::RegCallback()
{
	//��̬�࣬������������˴�����ѡ��дweak callback
	auto data_cb = [=](const QByteArray& bytes, int64_t timestamp)
	{
		WriteByteArray(bytes, timestamp);
	};
	audio_decoder_.RegDataCallback(data_cb);
}

void AudioPlayerCore::Close()
{
}

void AudioPlayerCore::WriteByteArray(const QByteArray& byte_array, int64_t timestamp)
{
	if (io_ && !b_audio_seek_)
	{
		io_->Write(byte_array,timestamp);
		auto state = output_->state();
		if(output_->state() == QAudio::State::StoppedState)
		{
			emit SignalStart();
		}
	}
}

void AudioPlayerCore::SeekContinue()
{
	if (io_)
	{
		io_->Clear();
		audio_decoder_.Seek(seek_time_, seek_res_callback_);
		seek_res_callback_ = nullptr;
	}
	if (output_) 
	{
		b_audio_seek_ = false;
		output_->resume();
	}
}

int64_t AudioPlayerCore::GetCurrentTimestamp()
{
	if (io_) 
	{
		return io_->GetCurrentTimeStamp();
	}
	return 0;
}

void AudioPlayerCore::Pause()
{
	if(output_)
	{
		output_->suspend();
	}
}

void AudioPlayerCore::Resume()
{
	if(output_)
	{
		output_->resume();
	}
}

bool AudioPlayerCore::IsPaused()
{
	if(output_)
	{
		return output_->state() == QAudio::State::SuspendedState;
	}
	return true;
}

void AudioPlayerCore::Clear()
{
	if(io_)
	{
		io_->Clear();
	}
}

void AudioPlayerCore::Seek(int64_t timestamp,SeekResCallback res_cb)
{
	if(output_)
	{
		seek_res_callback_ = res_cb;
		b_audio_seek_ = true;
		seek_time_ = timestamp;
		output_->suspend();
	}

}
