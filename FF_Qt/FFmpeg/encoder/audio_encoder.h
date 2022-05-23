#pragma once
#include "memory"
class EncoderCriticalSec;
class AVStream;
class AudioEncoder 
{
public:
	AudioEncoder();
	~AudioEncoder();
	void Init(const std::weak_ptr<EncoderCriticalSec>& encoder_infos_);

private:
	bool AddAudioStream();
	bool OpenAudio();

private:
	std::weak_ptr<EncoderCriticalSec> encoder_infos_;
	AVStream* audio_stream_;
};