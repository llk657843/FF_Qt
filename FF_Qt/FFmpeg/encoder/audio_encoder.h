#pragma once
#include "memory"
#include "qbytearray.h"
class EncoderCriticalSec;
class AVStream;
class AudioEncoder 
{
public:
	AudioEncoder();
	~AudioEncoder();
	void Init(const std::weak_ptr<EncoderCriticalSec>& encoder_infos_);
	void PushBytes(const QByteArray& bytes,int64_t timestamp_ms);

private:
	bool AddAudioStream();
	bool OpenAudio();

private:
	std::weak_ptr<EncoderCriticalSec> encoder_infos_;
	AVStream* audio_stream_;
	int channel_cnt_;
};