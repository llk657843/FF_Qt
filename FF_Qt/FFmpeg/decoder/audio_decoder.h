#pragma once
#include <qbytearray.h>
#include "base_decoder.h"
#include "decoder_callback.h"
#include "../../Thread/threadsafe_queue.h"
class AVFormatContext;
class AVFrame;
class SwrContext;
class AVCodecContext;
class AVPacket;
class AudioPlayerCore;
using DataCallback = std::function<void(const QByteArray& bytes,int64_t timestamp)>;
using StopDecodeResCallback = std::function<void()>;
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
	void AsyncStop(StopDecodeResCallback);

private:
	void ReleaseAll();

private:
	int audio_stream_id_;
	AVCodecContext* av_codec_context_;
	SwrContext* swr_context_;
	int channel_cnt_;
	AVPacket* packet_;
	DataCallback data_cb_;
	int out_sample_rate_;
	StopDecodeResCallback stop_decode_cb_;
	std::atomic_bool stop_flag_;
	uint8_t* out_buffer_;
	std::atomic_bool b_running_;
};