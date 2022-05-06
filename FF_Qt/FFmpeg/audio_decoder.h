#pragma once
#include "audio_unit_param.h"
#include "base_decoder.h"
#include "../Thread/threadsafe_queue.h"
class AVFormatContext;
class AVFrame;
class SwrContext;
class AVCodecContext;
class AVPacket;
class AudioPlayerCore;
using DataCallback = std::function<void(const QByteArray& bytes,int64_t timestamp)>;
class AudioDecoder : public BaseDecoder
{
public:
	AudioDecoder();
	~AudioDecoder();
	bool Init(const std::string& path) override;
	bool Run() override;

	void RegDataCallback(DataCallback);
	void NotifyDataCallback(const QByteArray& bytes, int64_t timestamp);

	bool GetData(AudioUnitParam&);

private:
	int audio_stream_id_;
	AVCodecContext* av_codec_context_;
	thread_safe_queue<AudioUnitParam> buffer_;
	SwrContext* swr_context_;
	int channel_cnt_;
	AVPacket* packet_;
	DataCallback data_cb_;
};