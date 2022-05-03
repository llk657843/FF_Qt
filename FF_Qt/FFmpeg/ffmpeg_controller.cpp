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
#include "../time_strategy/time_base_define.h"
const int MAX_AUDIO_FRAME_SIZE = 48000 * 2 * 16 * 0.125;
FFMpegController::FFMpegController()
{
	format_context_ = nullptr;
	av_input_ = nullptr;
	open_done_callback_ = nullptr;
	fail_cb_ = nullptr;
	image_cb_ = nullptr;
	audio_player_core_ = nullptr;
	image_frames_.set_max_size(200);
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

ImageInfo* FFMpegController::PostImageTask(SwsContext* sws_context, AVFrame* frame, int width, int height,int64_t timestamp,QImage* output)
{
	//kThreadVideoRender
	ImageInfo* image_info = nullptr;
	if (frame)
	{
		int output_line_size[4];
		
		av_image_fill_linesizes(output_line_size, AV_PIX_FMT_ARGB, width);
		uint8_t* output_dst[] = { output->bits() };

		sws_scale(sws_context, frame->data, frame->linesize, 0, height, output_dst, output_line_size);
		FreeFrame(frame);
		image_info = new ImageInfo(timestamp,std::move(*output));
		delete output;
		return image_info;
	}
	return nullptr;
}

void FFMpegController::FreeFrame(AVFrame* ptr)
{
	av_frame_free(&ptr);
}

void FFMpegController::InitVideoDecoderFormat(VideoDecoderFormat& decoder_format)
{
	decoder_format.video_stream_index_ = av_find_best_stream(format_context_, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	decoder_format.codec_context_ = format_context_->streams[decoder_format.video_stream_index_]->codec;
	AVCodecID id = decoder_format.codec_context_->codec_id;
	AVCodec* av_decoder = avcodec_find_decoder(id);
	if (!av_decoder)
	{
		CallFail(-1, "find decoder failed");
		return;
	}

	avcodec_open2(decoder_format.codec_context_, av_decoder, NULL);
	frame_time_ = 1000.0 /format_context_->streams[decoder_format.video_stream_index_]->avg_frame_rate.num;

	decoder_format.width_ = decoder_format.codec_context_->width;
	decoder_format.height_= decoder_format.codec_context_->height;

	AVPixelFormat src_fmt = decoder_format.codec_context_->pix_fmt;

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
			
			QImage* output = new QImage(video_decoder_format.width_, video_decoder_format.height_, QImage::Format_ARGB32);
			auto func = [=]()
			{
				int64_t timestamp = frame->best_effort_timestamp * av_q2d(video_decoder_format.codec_context_->time_base) * 1000.0;
				return PostImageTask(video_decoder_format.context_, frame, video_decoder_format.width_, video_decoder_format.height_, timestamp, output);
			};
			image_frames_.push_back(func);
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
			int64_t res = audio_in_frame->best_effort_timestamp * 1000.0 * av_q2d(audio_decoder_format.avc_codec_context->pkt_timebase) ;
			audio_player_core_->WriteByteArray(byte_array, res);
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
	AVSampleFormat out_format = AVSampleFormat::AV_SAMPLE_FMT_S16;
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

void FFMpegController::Parse(AVFormatContext*& context,bool b_internal)
{
	if(path_.empty())
	{
		CallFail(-1,"media path empty");
	}
	if(context)
	{
		return;
	}

	int res = avformat_open_input(&context, path_.c_str(),av_input_,NULL);
	if(res != 0)
	{
		CallFail(res,"avformat open input failed");
		return;
	}

	res = avformat_find_stream_info(context,NULL);
	if(res != 0)
	{
		CallFail(res,"find stream info failed");
		return;
	}
	int64_t dur = context->duration;
	printf("Parse completed,duration :%lld\n", dur);
	av_dump_format(context, NULL, path_.c_str(), 0);

	//外部查询，则查完了就关掉
	if (!b_internal) 
	{
		if (context)
		{
			avformat_free_context(context);
			context = nullptr;
		}
	}
}

void FFMpegController::PauseAudio()
{
	if(audio_player_core_)
	{
		audio_player_core_->Pause();
	}
}

void FFMpegController::ResumeAudio()
{
	if(audio_player_core_)
	{
		audio_player_core_->Resume();
	}
}

void FFMpegController::AsyncOpen()
{
	Parse(format_context_,true);
	InitAudioPlayerCore();
	audio_player_core_->Play();
	auto video_task = [=]()
	{
		DecodeAll();
	};
	qtbase::Post2Task(kThreadDecoder, video_task);
}

bool FFMpegController::GetImage(ImageInfo*& image_info)
{
	//kThreadVideoRender
	DelayFunc delay_func;
	bool b_get = image_frames_.get_front_read_write(delay_func);
	int normal_wait_time = frame_time_ - BASE_SLEEP_TIME;
	if (b_get) {
		image_info = delay_func();
		int64_t audio_timestamp = 0;
		audio_timestamp = audio_player_core_->GetCurrentTimestamp();
		if (audio_timestamp < 30)
		{
			//音频可能还没开始跑，先休息一下
			Sleep(audio_timestamp);
		}
		else if (image_info && image_info->timestamp_ > audio_timestamp)
		{
			//视频比音频快，则让视频渲染等一会儿再渲染
			int64_t sleep_time = image_info->timestamp_ - audio_timestamp;
			sleep_time = sleep_time > MAX_ADJUST_TIME ? MAX_ADJUST_TIME + normal_wait_time : sleep_time + normal_wait_time;
			if (sleep_time > 0)
			{
				Sleep(sleep_time);
			}
			return true;
		}
		else if (image_info && image_info->timestamp_ < audio_timestamp)
		{
			//视频比音频慢，则让视频渲染加快
			int sub_time = audio_timestamp - image_info->timestamp_;
			sub_time = sub_time > MAX_ADJUST_TIME ? MAX_ADJUST_TIME : sub_time;
			normal_wait_time -= sub_time;
			if(normal_wait_time > 0)
			{
				Sleep(normal_wait_time);
			}
			return true;
		}
		else if(image_info && image_info->timestamp_ <= audio_timestamp)
		{
			Sleep(normal_wait_time);
			return true;
		}
		else
		{
			return false;
		}
	}
	return false;
}

void FFMpegController::RegImageCallback(ImageCallback image_cb)
{
	image_cb_ = image_cb;
}

void FFMpegController::RegOpenDoneCallback(OpenDoneCallback cb)
{
	open_done_callback_ = cb;
}
