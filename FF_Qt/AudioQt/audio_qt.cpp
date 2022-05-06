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
	sample_rate_ = 44100;
	connect(this,SIGNAL(SignalStart()),this,SLOT(SlotStart()));
	RegCallback();
}

AudioPlayerCore::~AudioPlayerCore()
{
}

void AudioPlayerCore::SetSamplerate(int sample_rate)
{
	sample_rate_ = sample_rate;
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
}

void AudioPlayerCore::SlotStart()
{
	static std::once_flag once_flag;
	std::call_once(once_flag, [=]()
	{
		output_->start(io_);//���ſ�ʼ
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
		
	}
}

void AudioPlayerCore::StartLoopReadBytes()
{
	
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
