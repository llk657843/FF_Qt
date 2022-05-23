#include "audio_encoder.h"
#include "iostream"
#include "define/encoder_critical_sec.h"
extern "C" 
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"

}
AudioEncoder::AudioEncoder()
{
	audio_stream_ = nullptr;
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
		return;
	}
	OpenAudio();
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
	codec_ctx->codec_id = AVCodecID::AV_CODEC_ID_AAC;
	codec_ctx->codec_type = AVMEDIA_TYPE_AUDIO;
	// Set format
	codec_ctx->bit_rate = 128000;
	codec_ctx->sample_rate = 44100;
	codec_ctx->channels = 2;
	codec_ctx->sample_fmt = AV_SAMPLE_FMT_S16;
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
