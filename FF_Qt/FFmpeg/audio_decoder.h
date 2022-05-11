#pragma once
#include <qbytearray.h>
#include "base_decoder.h"
#include "decoder_callback.h"
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
	int GetSamplerate() const;
	void Seek(int64_t seek_time, SeekResCallback);

private:
	int audio_stream_id_;
	AVCodecContext* av_codec_context_;
	SwrContext* swr_context_;
	int channel_cnt_;
	AVPacket* packet_;
	DataCallback data_cb_;
	int out_sample_rate_;
};