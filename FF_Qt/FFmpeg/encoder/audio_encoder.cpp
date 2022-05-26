#include "audio_encoder.h"
#include "iostream"
#include "define/encoder_critical_sec.h"
#include "../decoder/AVFrameWrapper.h"
#include "algorithm"
extern "C" 
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include <libswresample/swresample.h>
}
const int MAX_AUDIO_FRAME_SIZE = 48000 * 2 * 16 * 0.125;
AudioEncoder::AudioEncoder()
{
	channel_cnt_ = 2;
	audio_stream_ = nullptr;
	swr_context_ = nullptr;
}

AudioEncoder::~AudioEncoder()
{
}

void AudioEncoder::Init(const std::weak_ptr<EncoderCriticalSec>& encoder_infos)
{
	if (!encoder_infos.lock())
	{
		std::cout << "init encoder info first" << std::endl;
		return;
	}
	encoder_infos_ = encoder_infos;
	if (!AddAudioStream()) 
	{
		std::cout << "add audio stream failed" << std::endl;
		return;
	}
	OpenAudio();
	InitResample();
}

void AudioEncoder::PushBytes(const QByteArray& bytes, int64_t timestamp_ms)
{
	if (!audio_stream_) 
	{
		return;
	}
	AVCodecContext* ctx = audio_stream_->codec;
	std::shared_ptr<AVFrameWrapper> dst_frame = CreateFrame(AV_SAMPLE_FMT_FLTP,bytes,false);
	std::shared_ptr<AVFrameWrapper> src_frame = CreateFrame(AV_SAMPLE_FMT_S16, bytes,true);
	static int64_t last_frame = 0;
	//if(timestamp_ms == 0)
	//{
	//	dst_frame->Frame()->pts = 0;
	//	src_frame->Frame()->pts = 0;
	//	
	//}
	//else 
	//{
	//	dst_frame->Frame()->pts = last_frame + av_rescale_q(dst_frame->Frame()->nb_samples, ctx->time_base, audio_stream_->time_base);
	//	src_frame->Frame()->pts = last_frame + av_rescale_q(src_frame->Frame()->nb_samples, ctx->time_base, audio_stream_->time_base);
	//	last_frame = dst_frame->Frame()->pts;
	//	
	//}
	//dst_frame->Frame()->best_effort_timestamp = dst_frame->Frame()->pts;
	//src_frame->Frame()->best_effort_timestamp = src_frame->Frame()->pts;
	//std::cout << "last frame time :" << last_frame <<std::endl;
	//dst_frame->Frame()->pkt_dts = dst_frame->Frame()->pts;
	//src_frame->Frame()->pkt_dts = src_frame->Frame()->pts;
	int res = swr_convert(swr_context_, dst_frame->Frame()->data, MAX_AUDIO_FRAME_SIZE,
		(const uint8_t **)src_frame->Frame()->data, src_frame->Frame()->nb_samples);
	if (res < 0) 
	{
		std::cout << "convert audio failed" << std::endl;
		return;
	}
	dst_frame->Frame()->pts = last_frame++;
	res = avcodec_send_frame(ctx, dst_frame->Frame());
	if(res != 0)
	{
		std::cout << "send audio frame failed" << std::endl;
		return;
	}
	AVPacketWrapper av_packet;
	av_packet.Init();
	av_packet.Get()->flags |= AV_PKT_FLAG_KEY;
	av_packet.Get()->stream_index = audio_stream_->index;
	res = avcodec_receive_packet(ctx,av_packet.Get());
	if(res != 0)
	{
		return;
	}
	std::shared_ptr<EncoderCriticalSec> shared_info = encoder_infos_.lock();
	if(shared_info)
	{
		shared_info->WriteFrame(av_packet);
	}
}

bool AudioEncoder::AddAudioStream()
{
	auto encoder_ptr = encoder_infos_.lock();
	if(!encoder_ptr)
	{
		return false;
	}

	// Try create stream.
	audio_stream_ = encoder_ptr->CreateNewStream();
	if (!audio_stream_)
	{
		printf("Cannot add new audio stream\n");
		return false;
	}

	// Codec.
	AVCodecContext* codec_ctx = audio_stream_->codec;
	//codec_ctx->codec_id = AVCodecID::AV_CODEC_ID_PCM_S16LE;
	codec_ctx->codec_id = (AVCodecID)encoder_ptr->GetAudioCodecId();
	codec_ctx->codec_type = AVMEDIA_TYPE_AUDIO;
	// Set format
	codec_ctx->bit_rate = 128000;
	codec_ctx->sample_rate = 44100;
	codec_ctx->channels = 2;
	codec_ctx->channel_layout = AV_CH_LAYOUT_STEREO;
	codec_ctx->sample_fmt = AV_SAMPLE_FMT_FLTP;
	// Some formats want stream headers to be separate.
	codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	return true;
}

bool AudioEncoder::OpenAudio()
{
	AVCodecContext* codec_ctx = NULL;
	AVCodec* pCodec = NULL;
	codec_ctx = audio_stream_->codec;

	// Find the audio encoder.
	pCodec = avcodec_find_encoder(codec_ctx->codec_id);
	if (!pCodec)
	{
		printf("Cannot open audio codec\n");
		return false;
	}
	// Open it.
	if (avcodec_open2(codec_ctx, pCodec, NULL) < 0)
	{
		printf("Cannot open audio codec\n");
		return false;
	}
	return true;
}

void AudioEncoder::InitResample()
{
	swr_context_ = swr_alloc();
	//音频格式  输入的采样设置参数
	AVSampleFormat in_format = AVSampleFormat::AV_SAMPLE_FMT_S16;
	AVSampleFormat out_format = AVSampleFormat::AV_SAMPLE_FMT_FLTP;
	int in_sample_rate = 44100;
	int out_sample_rate = 44100;
	uint64_t in_ch_layout = AV_CH_LAYOUT_STEREO;
	uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;

	//给Swrcontext 分配空间，设置公共参数
	swr_alloc_set_opts(swr_context_, out_ch_layout, out_format, out_sample_rate,
		in_ch_layout, in_format, in_sample_rate, 0, NULL
	);
	// 初始化
	swr_init(swr_context_);
}

std::shared_ptr<AVFrameWrapper> AudioEncoder::CreateFrame(AVSampleFormat sample_fmt, const QByteArray& bytes, bool b_fill_bytes)
{
	std::shared_ptr<AVFrameWrapper> frame_wrapper = std::make_shared<AVFrameWrapper>();
	AVCodecContext* ctx = audio_stream_->codec;
	int res = 0;
	if (b_fill_bytes) 
	{
		frame_wrapper->Frame()->nb_samples = ctx->frame_size;
		frame_wrapper->Frame()->channels = 2;
		frame_wrapper->Frame()->channel_layout = AV_CH_LAYOUT_STEREO;
		frame_wrapper->Frame()->format = sample_fmt;
		res = avcodec_fill_audio_frame(frame_wrapper->Frame(), channel_cnt_, sample_fmt, (const uint8_t*)bytes.data(), bytes.size(), 1);
		frame_wrapper->SetManualFree(true);
	}
	else 
	{
		frame_wrapper->Frame()->nb_samples = 1024;
		frame_wrapper->Frame()->channels = 2;
		frame_wrapper->Frame()->format = sample_fmt;
		frame_wrapper->Frame()->channel_layout = AV_CH_LAYOUT_STEREO;
		res = av_frame_get_buffer(frame_wrapper->Frame(), 0);
		frame_wrapper->SetManualFree(true);
		if(res < 0)
		{
			std::cout << "frame init failed" << std::endl;
		}
	}

	if (res < 0)
	{
		std::cout << "fill audio frame failed" << std::endl;
	}
	return frame_wrapper;
}
