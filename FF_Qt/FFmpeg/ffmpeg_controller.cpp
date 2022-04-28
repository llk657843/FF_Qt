#include "ffmpeg_controller.h"

#include <iostream>
#include <qimage.h>
#include <QThread>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/imgutils.h"
}
#include "windows.h"
#include "../Thread/thread_pool_entrance.h"
#include "../AudioQt/audio_qt.h"
#include "../Audio/mq_manager.h"
const int MAX_AUDIO_FRAME_SIZE = 48000 * 2 * 16 * 0.125;
FFMpegController::FFMpegController()
{
	format_context_ = nullptr;
	av_input_ = nullptr;
	open_done_callback_ = nullptr;
	fail_cb_ = nullptr;
	image_cb_ = nullptr;
	audio_player_core_ = nullptr;
	images_.set_max_size(100);
	InitSdk();
}

FFMpegController::~FFMpegController()
{
	Close();
}

void FFMpegController::Init(const std::string& path)
{
	path_ = path;
}

void FFMpegController::RegFailCallback(FailCallback fail_cb)
{
	fail_cb_ = fail_cb;
}

void FFMpegController::Close()
{
	if(av_input_)
	{
		av_free(av_input_);
		av_input_ = nullptr;
	}
	if(format_context_)
	{
		avformat_free_context(format_context_);
		format_context_ = nullptr;
	}

}

void FFMpegController::CallOpenDone()
{
	if(open_done_callback_)
	{
		open_done_callback_();
	}
}

void FFMpegController::InitAudioPlayerCore()
{
	if(audio_player_core_)
	{
		return;
	}
	audio_player_core_ = new AudioPlayerCore;
	if (format_context_) 
	{
		audio_player_core_->SetSamplerate(format_context_->streams[AVMEDIA_TYPE_AUDIO]->codec->sample_rate);
	}
}

void FFMpegController::DecodeAll()
{
	//解析视频相关参数
	VideoDecoderFormat video_decoder_format;
	InitVideoDecoderFormat(video_decoder_format);
	//解析音频相关参数
	AudioDecoderFormat audio_decoder_format;
	InitAudioDecoderFormat(audio_decoder_format);
	//开始解析
	DecodeCore(video_decoder_format, audio_decoder_format);
	//释放资源
	Close();
}

void FFMpegController::PostImageTask(SwsContext* sws_context,AVFrame* frame,int width,int height)
{
	auto image_task = [=]()
	{
		if (frame)
		{
			int output_line_size[4];
			QImage output(width, height, QImage::Format_ARGB32);
			av_image_fill_linesizes(output_line_size, AV_PIX_FMT_ARGB, width);
			uint8_t* output_dst[] = { output.bits() };
			sws_scale(sws_context, frame->data, frame->linesize, 0, height, output_dst, output_line_size);
			images_.push_back(output);
			FreeFrame(frame);
		}
	};
	qtbase::Post2Task(kThreadVideoDecoder, image_task);
}

void FFMpegController::FreeFrame(AVFrame* ptr)
{
	av_frame_free(&ptr);
}

void FFMpegController::InitVideoDecoderFormat(VideoDecoderFormat& decoder_format)
{
	decoder_format.video_stream_index_ = av_find_best_stream(format_context_, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	AVCodecID id = format_context_->streams[decoder_format.video_stream_index_]->codec->codec_id;
	AVCodec* av_decoder = avcodec_find_decoder(id);
	if (!av_decoder)
	{
		CallFail(-1, "find decoder failed");
		return;
	}
	avcodec_open2(format_context_->streams[decoder_format.video_stream_index_]->codec, av_decoder, NULL);
	frame_ = format_context_->streams[decoder_format.video_stream_index_]->avg_frame_rate.num;

	decoder_format.width_ = format_context_->streams[decoder_format.video_stream_index_]->codec->width;
	decoder_format.height_= format_context_->streams[decoder_format.video_stream_index_]->codec->height;
	AVPixelFormat src_fmt = format_context_->streams[decoder_format.video_stream_index_]->codec->pix_fmt;

	decoder_format.context_ = sws_getContext(decoder_format.width_, decoder_format.height_, src_fmt, decoder_format.width_, decoder_format.height_, AVPixelFormat::AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);
	CallOpenDone();
}

void FFMpegController::DecodeCore(VideoDecoderFormat& video_decoder_format, AudioDecoderFormat& audio_decoder_format)
{
	AVFrame* audio_in_frame = av_frame_alloc();
	//开始读取源文件，进行解码
	AVPacket* packet = (AVPacket*)av_malloc(sizeof(AVPacket));
	// 设置音频缓冲区间 16bit   44100  PCM数据, 双声道
	uint8_t* out_buffer = (uint8_t*)av_malloc(MAX_AUDIO_FRAME_SIZE);
	while (av_read_frame(format_context_, packet) >= 0)
	{
		if (packet->stream_index == AVMEDIA_TYPE_VIDEO)
		{
			if (avcodec_send_packet(format_context_->streams[video_decoder_format.video_stream_index_]->codec, packet) != 0)
			{
				continue;
			}
			AVFrame* frame = av_frame_alloc();
			if (avcodec_receive_frame(format_context_->streams[video_decoder_format.video_stream_index_]->codec, frame) != 0)
			{
				av_frame_free(&frame);
				continue;
			}
			PostImageTask(video_decoder_format.context_, frame, video_decoder_format.width_, video_decoder_format.height_);
			Sleep(20);
		}

		else if (packet->stream_index == audio_decoder_format.audio_stream_index_)
		{
			avcodec_send_packet(audio_decoder_format.avc_codec_context, packet);
			//解码
			if (avcodec_receive_frame(audio_decoder_format.avc_codec_context, audio_in_frame) != 0)
			{
				continue;
			}
			//将每一帧数据转换成pcm
			swr_convert(audio_decoder_format.swr_context_, &out_buffer, MAX_AUDIO_FRAME_SIZE,
				(const uint8_t**)audio_in_frame->data, audio_in_frame->nb_samples);
			//获取实际的缓存大小
			int out_buffer_size = av_samples_get_buffer_size(NULL, audio_decoder_format.out_channel_cnt_, audio_in_frame->nb_samples, AV_SAMPLE_FMT_S16, 1);
			// 写入文件
			QByteArray byte_array;
			byte_array.append((char*)out_buffer, out_buffer_size);
			audio_player_core_->WriteByteArray(byte_array);
		}
	}
	av_free(audio_decoder_format.swr_context_);
}

void FFMpegController::InitAudioDecoderFormat(AudioDecoderFormat& audio_decoder)
{
	audio_decoder.audio_stream_index_ = av_find_best_stream(format_context_, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	audio_decoder.avc_codec_context = format_context_->streams[audio_decoder.audio_stream_index_]->codec;
	AVCodecID audio_id = audio_decoder.avc_codec_context->codec_id;
	AVCodec* av_audio_decoder = avcodec_find_decoder(audio_id);

	if (!av_audio_decoder)
	{
		CallFail(-1, "find decoder failed");
		return;
	}
	int ret = avcodec_open2(audio_decoder.avc_codec_context, av_audio_decoder, NULL);
	if (ret < 0)
	{
		CallFail(-1, "av decoder failed");
		return;
	}
	audio_decoder.swr_context_ = swr_alloc();
	//音频格式  输入的采样设置参数
	AVSampleFormat in_format = audio_decoder.avc_codec_context->sample_fmt;
	AVSampleFormat out_format = AV_SAMPLE_FMT_S16;
	int in_sample_rate = audio_decoder.avc_codec_context->sample_rate;
	int out_sample_rate = audio_decoder.avc_codec_context->sample_rate;
	uint64_t in_ch_layout = audio_decoder.avc_codec_context->channel_layout;
	uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;

	//给Swrcontext 分配空间，设置公共参数
	swr_alloc_set_opts(audio_decoder.swr_context_, out_ch_layout, out_format, out_sample_rate,
		in_ch_layout, in_format, in_sample_rate, 0, NULL
	);
	// 初始化
	swr_init(audio_decoder.swr_context_);
	
	// 获取声道数量
	audio_decoder.out_channel_cnt_ = av_get_channel_layout_nb_channels(out_ch_layout);
}

void FFMpegController::CallFail(int code, const std::string& msg)
{
	if(!msg.empty())
	{
		std::cout << msg << std::endl;
	}
	Close();
	if(fail_cb_)
	{
		fail_cb_(code, msg);
	}
}

void FFMpegController::InitSdk()
{
	av_register_all();
}

void FFMpegController::Parse(bool b_internal)
{
	if(path_.empty())
	{
		CallFail(-1,"media path empty");
	}

	int res = avformat_open_input(&format_context_, path_.c_str(),av_input_,NULL);
	if(res != 0)
	{
		CallFail(res,"avformat open input failed");
		return;
	}

	res = avformat_find_stream_info(format_context_,NULL);
	if(res != 0)
	{
		CallFail(res,"find stream info failed");
		return;
	}
	int64_t dur = format_context_->duration;
	printf("Parse completed,duration :%lld\n", dur);
	av_dump_format(format_context_, NULL, path_.c_str(), 0);

	//外部查询，则查完了就关掉
	if (!b_internal) 
	{
		Close();
	}
}

void FFMpegController::AsyncOpen()
{
	Parse(true);
	InitAudioPlayerCore();
	auto video_task = [=]()
	{
		DecodeAll();
	};
	audio_player_core_->Play();
	qtbase::Post2Task(kThreadHTTP, video_task);
}

bool FFMpegController::GetImage(QImage& image)
{
	return images_.get_front_read_write(std::forward<QImage&>(image));
}

void FFMpegController::RegImageCallback(ImageCallback image_cb)
{
	image_cb_ = image_cb;
}

void FFMpegController::RegOpenDoneCallback(OpenDoneCallback cb)
{
	open_done_callback_ = cb;
}
