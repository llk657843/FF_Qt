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
}

void AudioPlayerCore::SlotStart()
{
	std::once_flag once_flag;
	std::call_once(once_flag, [=]()
	{
		output_->start(io_);//播放开始
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
