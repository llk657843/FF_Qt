#include "audio_decoder.h"

#include <iostream>
#include <ostream>
#include <qbytearray.h>

#include "../AudioQt/audio_qt.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}
const int MAX_AUDIO_FRAME_SIZE = 48000 * 2 * 16 * 0.125;
AudioDecoder::AudioDecoder()
{
	data_cb_ = nullptr;
	channel_cnt_ = 0;
	out_sample_rate_ = 0;
	buffer_.set_max_size(200); //最多含200个缓存，约10秒缓存
}

AudioDecoder::~AudioDecoder()
{
}

bool AudioDecoder::Init(const std::string& path)
{
	bool b_success = PrepareDeocde(path);
	if(!b_success)
	{
		return false;
	}
	audio_stream_id_ = av_find_best_stream(decoder_, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	AVCodecParameters* codec_param = decoder_->streams[audio_stream_id_]->codecpar;

	//创建编码器上下文
	av_codec_context_ = avcodec_alloc_context3(decoder_->audio_codec);
	if (!av_codec_context_)
	{
		return false;
	}

	//复制编码参数到上下文中
	int ret = avcodec_parameters_to_context(av_codec_context_, codec_param);
	if (ret != 0)
	{
		return false;
	}
	AVCodec* codec = avcodec_find_decoder(av_codec_context_->codec_id);
	if (avcodec_open2(av_codec_context_, codec, NULL) < 0)
	{
		return false;
	}
	swr_context_ = swr_alloc();
	//音频格式  输入的采样设置参数
	AVSampleFormat in_format = av_codec_context_->sample_fmt;
	AVSampleFormat out_format = AVSampleFormat::AV_SAMPLE_FMT_S16;
	int in_sample_rate = av_codec_context_->sample_rate;
	out_sample_rate_ = av_codec_context_->sample_rate;
	uint64_t in_ch_layout = av_codec_context_->channel_layout;
	uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;

	//给Swrcontext 分配空间，设置公共参数
	swr_alloc_set_opts(swr_context_, out_ch_layout, out_format, out_sample_rate_,
		in_ch_layout, in_format, in_sample_rate, 0, NULL
	);
	// 初始化
	swr_init(swr_context_);

	// 获取声道数量
	channel_cnt_ = av_get_channel_layout_nb_channels(out_ch_layout);
	return true;
}

bool AudioDecoder::Run()
{
	packet_ = (AVPacket*)av_malloc(sizeof(AVPacket));
	int64_t duration_time = 0;
	uint8_t* out_buffer = (uint8_t*)av_malloc(MAX_AUDIO_FRAME_SIZE);
	while (av_read_frame(decoder_, packet_) >= 0)
	{
		if (packet_->stream_index == audio_stream_id_)
		{
			AVFrame* audio_in_frame = av_frame_alloc();
			avcodec_send_packet(av_codec_context_, packet_);
			//解码
			if (avcodec_receive_frame(av_codec_context_, audio_in_frame) != 0)
			{
				av_frame_free(&audio_in_frame);
				continue;
			}
			//将每一帧数据转换成pcm
			swr_convert(swr_context_, &out_buffer, MAX_AUDIO_FRAME_SIZE,
				(const uint8_t**)audio_in_frame->data, audio_in_frame->nb_samples);
			//获取实际的缓存大小
			int out_buffer_size = av_samples_get_buffer_size(NULL,channel_cnt_, audio_in_frame->nb_samples, AV_SAMPLE_FMT_S16, 1);
			// 写入文件
			QByteArray byte_array;
			byte_array.append((char*)out_buffer, out_buffer_size);
			auto timebase = decoder_->streams[audio_stream_id_]->codec->pkt_timebase;
			duration_time = audio_in_frame->best_effort_timestamp * 1000.0 * av_q2d(timebase);
			av_frame_free(&audio_in_frame);
			NotifyDataCallback(byte_array,duration_time);
		}
	}
	av_free(packet_);
	return true;
}

void AudioDecoder::RegDataCallback(DataCallback cb)
{
	data_cb_ = cb;
}

void AudioDecoder::NotifyDataCallback(const QByteArray& bytes, int64_t timestamp)
{
	if(data_cb_)
	{
		data_cb_(bytes, timestamp);
	}
}

bool AudioDecoder::GetData(AudioUnitParam& audio_unit)
{
	if(!buffer_.is_empty_lock())
	{
		return buffer_.get_front_read_write(audio_unit);
	}
	return false;
}

int AudioDecoder::GetSamplerate() const
{
	return out_sample_rate_;
}
