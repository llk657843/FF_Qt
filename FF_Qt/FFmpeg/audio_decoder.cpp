#include "audio_decoder.h"

#include <iostream>
#include <ostream>
#include <qbytearray.h>

#include "AVFrameWrapper.h"
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
	//buffer_.set_max_size(200); //��ຬ200�����棬Լ10�뻺��
}

AudioDecoder::~AudioDecoder()
{
	if (decoder_) 
	{
		avformat_free_context(decoder_);
		decoder_ = nullptr;
	}
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

	//����������������
	av_codec_context_ = avcodec_alloc_context3(decoder_->audio_codec);
	if (!av_codec_context_)
	{
		return false;
	}

	//���Ʊ����������������
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
	//��Ƶ��ʽ  ����Ĳ������ò���
	AVSampleFormat in_format = av_codec_context_->sample_fmt;
	AVSampleFormat out_format = AVSampleFormat::AV_SAMPLE_FMT_S16;
	int in_sample_rate = av_codec_context_->sample_rate;
	out_sample_rate_ = av_codec_context_->sample_rate;
	uint64_t in_ch_layout = av_codec_context_->channel_layout;
	uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;

	//��Swrcontext ����ռ䣬���ù�������
	swr_alloc_set_opts(swr_context_, out_ch_layout, out_format, out_sample_rate_,
		in_ch_layout, in_format, in_sample_rate, 0, NULL
	);
	// ��ʼ��
	swr_init(swr_context_);

	// ��ȡ��������
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
			AVFrameWrapper frame_wrapper;
			avcodec_send_packet(av_codec_context_, packet_);
			//����
			if (avcodec_receive_frame(av_codec_context_, frame_wrapper.frame_) != 0)
			{
				av_packet_unref(packet_);
				continue;
			}
			//��ÿһ֡����ת����pcm
			swr_convert(swr_context_, &out_buffer, MAX_AUDIO_FRAME_SIZE,
				(const uint8_t**)frame_wrapper.frame_->data, frame_wrapper.frame_->nb_samples);
			//��ȡʵ�ʵĻ����С
			int out_buffer_size = av_samples_get_buffer_size(NULL,channel_cnt_, frame_wrapper.frame_->nb_samples, AV_SAMPLE_FMT_S16, 1);
			// д���ļ�
			QByteArray byte_array;
			byte_array.append((char*)out_buffer, out_buffer_size);
			auto timebase = decoder_->streams[audio_stream_id_]->codec->pkt_timebase;
			duration_time = frame_wrapper.frame_->best_effort_timestamp * 1000.0 * av_q2d(timebase);
			NotifyDataCallback(byte_array,duration_time);
		}
		av_packet_unref(packet_);
	}
	av_packet_free(&packet_);
	avformat_free_context(decoder_);
	decoder_ = nullptr;
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

int AudioDecoder::GetSamplerate() const
{
	return out_sample_rate_;
}

void AudioDecoder::Seek(int64_t seek_time)
{
	std::lock_guard<std::mutex> lock(decode_mutex_);

	//image_funcs_.clear();
	//image_funcs_.notify_one();
	/*auto time_base = codec_context_->time_base;
	int seek_frame = seek_time_ms / av_q2d(time_base) / 1000.0;
	av_seek_frame(decoder_, video_stream_id_, seek_frame, AVSEEK_FLAG_BACKWARD);*/
}
