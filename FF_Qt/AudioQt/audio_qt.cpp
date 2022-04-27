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
	connect(this,SIGNAL(SignalStart()),this,SLOT(SlotStart()));
}

AudioPlayerCore::~AudioPlayerCore()
{
}

void AudioPlayerCore::Play()
{
	QAudioFormat fmt;//设置音频输出格式
	fmt.setSampleRate(44100);//1秒的音频采样率
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

void AudioPlayerCore::WriteThread()
{
	int index = 0;
	bool b_end = false;
	int size = bytes_.size();
	int buffer_cnt = 200;
	io_->Write(bytes_);
	bytes_.clear();
}

void AudioPlayerCore::PlayFile()
{
	QAudioFormat fmt;//设置音频输出格式
	fmt.setSampleRate(44100);//1秒的音频采样率
	fmt.setSampleSize(16);//声音样本的大小
	fmt.setChannelCount(2);//声道
	fmt.setCodec("audio/pcm");//解码格式
	fmt.setByteOrder(QAudioFormat::LittleEndian);
	fmt.setSampleType(QAudioFormat::UnSignedInt);//设置音频类型
	output_ = new QAudioOutput(fmt);
	QFile file("F:/mojito.pcm");
	file.open(QIODevice::ReadOnly);
	bytes_ = file.readAll();
	file.close();
	io_ = new AudioIoDevice;
	io_->open(QIODevice::ReadWrite);
	
	WriteThread();
	emit SignalStart();
	auto task =ToWeakCallback( [=]()
	{
		printf("%lld\n",io_->pos());
	});

	qtbase::Post2RepeatedTask(kThreadHTTP,task,std::chrono::seconds(1));
}

void AudioPlayerCore::WriteByteArray(QByteArray byte_array)
{
	if (io_) 
	{
		io_->Write(byte_array);
		auto state = output_->state();
		if(output_->state() == QAudio::State::StoppedState && io_->GetDataSize() >= 50000)
		{
			emit SignalStart();
		}
	}
}
