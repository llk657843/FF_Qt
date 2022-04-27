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
	char_queue_.set_max_size(8000);
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
	output_->start(io_);//播放开始
}

void AudioPlayerCore::SlotStart()
{
	output_->start(io_);//播放开始
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
	while (!b_end)
	{
		int read_size = output_->periodSize();
		int chunks = 0;
		if (read_size > 0) 
		{
			chunks = output_->bytesFree() / read_size;
		}
	}
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
	
	auto delay_task = [=]()
	{
		emit SignalStart();
	};

	qtbase::Post2DelayedTask(kThreadAudioRender, delay_task, std::chrono::seconds(1));
	auto task = [=]()
	{
		WriteThread();
	};
	qtbase::Post2Task(kThreadUIHelper, task);
}