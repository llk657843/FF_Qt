#pragma once
#include "memory"
#include "qbytearray.h"
#include "../decoder/AVFrameWrapper.h"
extern "C" 
{
#include "libavutil\samplefmt.h"
}
class EncoderCriticalSec;
class AVStream;
class SwrContext;
class AudioEncoder 
{
public:
	AudioEncoder();
	~AudioEncoder();
	void Init(const std::weak_ptr<EncoderCriticalSec>& encoder_infos_);
	void PushBytes(const QByteArray& bytes);
	void Stop();

private:
	bool AddAudioStream();
	bool OpenAudio();
	void InitResample();
	std::shared_ptr<AVFrameWrapper> CreateFrame(AVSampleFormat sample_fmt,const QByteArray&,bool b_fill_bytes);
	bool SendFrame(std::shared_ptr<AVFrameWrapper> frame_wrapper);

private:
	std::weak_ptr<EncoderCriticalSec> encoder_infos_;
	AVStream* audio_stream_;
	int channel_cnt_;
	SwrContext* swr_context_;
	int last_frame_timestamp_;
	int64_t last_pts_;
	std::atomic_bool b_stop_;
};