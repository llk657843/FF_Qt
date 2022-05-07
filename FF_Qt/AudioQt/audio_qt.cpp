#include "audio_qt.h"
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
	b_stop_ = false;
	output_ = nullptr;
	io_ = nullptr;
	output_ = nullptr;
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
	QAudioFormat fmt;//设置音频输出格式
	fmt.setSampleRate(sample_rate_);//1秒的音频采样率
	fmt.setSampleSize(16);//声音样本的大小
	fmt.setChannelCount(2);//声道
	fmt.setCodec("audio/pcm");//解码格式
	fmt.setByteOrder(QAudioFormat::LittleEndian);
	fmt.setSampleType(QAudioFormat::UnSignedInt);//设置音频类型
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

void AudioPlayerCore::SlotStart()
{
	static std::once_flag once_flag;
	std::call_once(once_flag, [=]()
	{
		output_->start(io_);//播放开始
	});
}

void AudioPlayerCore::SlotStateChange(QAudio::State state)
{
	ViewCallback::GetInstance()->NotifyAudioStateCallback(state);
	if(state == QAudio::State::IdleState)
	{

	}
	if(state == QAudio::State::ActiveState)
	{
		NotifyPlayStartCallback();
	}
}

void AudioPlayerCore::RegCallback()
{
	//静态类，生命周期最长，此处可以选择不写weak callback
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
	if (io_) 
	{
		io_->Write(byte_array,timestamp);
		auto state = output_->state();
		if(output_->state() == QAudio::State::StoppedState)
		{
			emit SignalStart();
		}
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

void AudioPlayerCore::RegPlayStartCallback(PlayStartCallback cb)
{
	play_start_callback_ = cb;
}

void AudioPlayerCore::NotifyPlayStartCallback()
{
	if(play_start_callback_)
	{
		play_start_callback_();
	}
}
