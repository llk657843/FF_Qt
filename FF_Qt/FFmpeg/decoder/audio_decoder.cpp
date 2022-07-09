#include "audio_decoder.h"

#include <iostream>
#include <ostream>
#include <qbytearray.h>

#include "AVFrameWrapper.h"
#include "../../AudioQt/audio_qt.h"
#include "../../view_callback/view_callback.h"
extern "C"
{
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}
const int MAX_AUDIO_FRAME_SIZE = 48000 * 2 * 16 * 0.125;
AudioDecoder::AudioDecoder()
{
	av_codec_context_ = nullptr;
	data_cb_ = nullptr;
	channel_cnt_ = 0;
	out_sample_rate_ = 0; 
	stop_decode_cb_ = nullptr;
	stop_flag_ = false;
	out_buffer_ = nullptr;
	b_running_ = false;
	swr_context_ = nullptr;
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
	if (audio_stream_id_ < 0) 
	{
		return false;		
	}
	
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
	ViewCallback::GetInstance()->NotifyParseDone(decoder_->duration);
	return true;
}

bool AudioDecoder::Run()
{
	//b_running�жϺ����Ƿ�����ִ��
	packet_ = (AVPacket*)av_malloc(sizeof(AVPacket));
	int64_t duration_time = 0;
	out_buffer_ = (uint8_t*)av_malloc(MAX_AUDIO_FRAME_SIZE);
	int out_buffer_size = 0;
	b_running_ = true;
	while (!stop_flag_ && ReadFrame(packet_))
	{
		if (stop_flag_)
		{
			av_packet_unref(packet_);
			break;
		}
		if (packet_->stream_index == audio_stream_id_)
		{
			auto frame_wrapper = std::make_shared<AVFrameWrapper>();
			SendPacket(av_codec_context_, packet_);
			//����
			if (ReceiveFrame(av_codec_context_, frame_wrapper))
			{
				av_packet_unref(packet_);
				continue;
			}
			//��ÿһ֡����ת����pcm
			QByteArray byte_array;
			{
				std::lock_guard<std::mutex> lock(decode_mutex_);
				swr_convert(swr_context_, &out_buffer_, MAX_AUDIO_FRAME_SIZE,
					(const uint8_t**)frame_wrapper->Frame()->data, frame_wrapper->Frame()->nb_samples);
				//��ȡʵ�ʵĻ����С
				out_buffer_size = av_samples_get_buffer_size(NULL, channel_cnt_, frame_wrapper->Frame()->nb_samples, AV_SAMPLE_FMT_S16, 1);
				// д���ļ�
				byte_array.append((char*)out_buffer_, out_buffer_size);
				auto timebase = decoder_->streams[audio_stream_id_]->codec->pkt_timebase;
				duration_time = frame_wrapper->Frame()->best_effort_timestamp * 1000.0 * av_q2d(timebase);
				//std::cout << "duration_time:" << duration_time << std::endl;
			}
			
			NotifyDataCallback(byte_array,duration_time);
		}
		av_packet_unref(packet_);
	}
	ReleaseAll();
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

void AudioDecoder::Seek(int64_t seek_time, SeekResCallback res_cb)
{
	std::lock_guard<std::mutex> lock(decode_mutex_);
	if (decoder_) 
	{
		auto timebase = decoder_->streams[audio_stream_id_]->codec->pkt_timebase;
		int seek_frame = seek_time / av_q2d(timebase) / 1000.0;
		bool res = av_seek_frame(decoder_, audio_stream_id_, seek_frame, AVSEEK_FLAG_BACKWARD) >= 0 ? true : false;
		if(res_cb)
		{
			res_cb(seek_frame, audio_stream_id_,res);
		}
	}
	else 
	{
		if (res_cb)
		{
			res_cb(0, 0, false);
		}
	}
}

void AudioDecoder::AsyncStop(StopDecodeResCallback res_cb)
{
	stop_decode_cb_ = res_cb;
	//happens before
	stop_flag_ = true;
	
	if (!b_running_) 
	{
		ReleaseAll();
	}
}

void AudioDecoder::ReleaseAll()
{
	avcodec_free_context(&av_codec_context_);
	av_codec_context_ = nullptr;

	av_packet_free(&packet_);
	packet_ = nullptr;
	avformat_free_context(decoder_);
	decoder_ = nullptr;

	av_free(out_buffer_);
	out_buffer_ = NULL;

	swr_free(&swr_context_);
	swr_context_ = nullptr;

	stop_flag_ = false;
	b_running_ = false;
	if (stop_decode_cb_) 
	{
		stop_decode_cb_();
		stop_decode_cb_ = nullptr;
	}
}
