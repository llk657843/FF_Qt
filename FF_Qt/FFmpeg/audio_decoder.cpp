#include "audio_decoder.h"
#include <qbytearray.h>
extern "C"
{
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}
const int MAX_AUDIO_FRAME_SIZE = 48000 * 2 * 16 * 0.125;
AudioDecoder::AudioDecoder()
{
	channel_cnt_ = 0;
	buffer_.set_max_size(200); //��ຬ200�����棬Լ10�뻺��
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
	int out_sample_rate = av_codec_context_->sample_rate;
	uint64_t in_ch_layout = av_codec_context_->channel_layout;
	uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;

	//��Swrcontext ����ռ䣬���ù�������
	swr_alloc_set_opts(swr_context_, out_ch_layout, out_format, out_sample_rate,
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
			AVFrame* audio_in_frame = av_frame_alloc();
			avcodec_send_packet(av_codec_context_, packet_);
			//����
			if (avcodec_receive_frame(av_codec_context_, audio_in_frame) != 0)
			{
				av_frame_free(&audio_in_frame);
				continue;
			}
			//��ÿһ֡����ת����pcm
			swr_convert(swr_context_, &out_buffer, MAX_AUDIO_FRAME_SIZE,
				(const uint8_t**)audio_in_frame->data, audio_in_frame->nb_samples);
			//��ȡʵ�ʵĻ����С
			int out_buffer_size = av_samples_get_buffer_size(NULL,channel_cnt_, audio_in_frame->nb_samples, AV_SAMPLE_FMT_S16, 1);
			// д���ļ�
			QByteArray byte_array;
			byte_array.append((char*)out_buffer, out_buffer_size);
			duration_time = audio_in_frame->best_effort_timestamp * 1000.0 * av_q2d(av_codec_context_->pkt_timebase) ;
			av_frame_free(&audio_in_frame);
			buffer_.push_back(AudioUnitParam(std::forward<QByteArray>(byte_array),duration_time));
		}
	}
	av_free(packet_);
	return true;
}
