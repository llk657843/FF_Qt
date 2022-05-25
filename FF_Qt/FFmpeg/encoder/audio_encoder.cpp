#include "audio_encoder.h"
#include "iostream"
#include "define/encoder_critical_sec.h"
#include "../decoder/AVFrameWrapper.h"
#include "algorithm"
extern "C" 
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"

}
AudioEncoder::AudioEncoder()
{
	channel_cnt_ = 2;
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

void AudioEncoder::PushBytes(const QByteArray& bytes, int64_t timestamp_ms)
{
	if (!audio_stream_) 
	{
		return;
	}
	AVCodecContext* ctx = audio_stream_->codec;
	int size = bytes.size();
	std::shared_ptr<AVFrameWrapper> frame_wrapper = std::make_shared<AVFrameWrapper>();
	
	int sample_cnt = size / av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);	//样本数量
	int normal_sample_cnt = ctx->frame_size / av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
	frame_wrapper->Frame()->nb_samples =  normal_sample_cnt < sample_cnt ? normal_sample_cnt : sample_cnt;
	int res = avcodec_fill_audio_frame(frame_wrapper->Frame(), channel_cnt_,AVSampleFormat::AV_SAMPLE_FMT_S16,(const uint8_t*)bytes.data(),bytes.size(),1);
	if (res < 0) 
	{
		std::cout << "fill audio frame failed" << std::endl;
		return;
	}
	res = avcodec_send_frame(ctx, frame_wrapper->Frame());
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
		std::cout << "receive audio frame failed" << std::endl;
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
	codec_ctx->codec_id = AVCodecID::AV_CODEC_ID_PCM_S16LE;
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
