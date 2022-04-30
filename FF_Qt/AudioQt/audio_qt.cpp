#include "audio_qt.h"
#include <QAudioOutput>
#include "QIODevice"
#include "typeinfo"
#include "../Thread/thread_pool_entrance.h"
#include "audio_io_device.h"
#include "QFile"
AudioPlayerCore::AudioPlayerCore()
{
	output_ = nullptr;
	sample_rate_ = 44100;
	connect(this,SIGNAL(SignalStart()),this,SLOT(SlotStart()));
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
}

void AudioPlayerCore::SlotStart()
{
	std::once_flag once_flag;
	std::call_once(once_flag, [=]()
	{
		output_->start(io_);//���ſ�ʼ
	});
}

void AudioPlayerCore::InitAudioFormat()
{

}

void AudioPlayerCore::WriteByteArray(QByteArray& byte_array, int64_t timestamp)
{
	static int cnt = 0;
	if (io_) 
	{
		cnt++;
		io_->Write(byte_array,timestamp);
		auto state = output_->state();
		if(output_->state() == QAudio::State::StoppedState && cnt>=2)
		{
			emit SignalStart();
		}
	}
}
