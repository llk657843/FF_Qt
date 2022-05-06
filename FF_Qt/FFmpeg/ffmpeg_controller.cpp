#include "ffmpeg_controller.h"
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
#include "../view_callback/view_callback.h"
const int MAX_AUDIO_FRAME_SIZE = 48000 * 2 * 16 * 0.125;
FFMpegController::FFMpegController()
{
	av_input_ = nullptr;
	open_done_callback_ = nullptr;
	fail_cb_ = nullptr;
	image_cb_ = nullptr;
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
	//if(format_context_)
	//{
	//	avformat_free_context(format_context_);
	//	format_context_ = nullptr;
	//}

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
	/*if(audio_player_core_)
	{
		return;
	}
	audio_player_core_ = new AudioPlayerCore;*/
	/*if (format_context_) 
	{
		audio_player_core_->SetSamplerate(format_context_->streams[AVMEDIA_TYPE_AUDIO]->codec->sample_rate);
	}*/
}

void FFMpegController::DecodeAll()
{
	//解析音频相关参数
	AudioDecoderFormat audio_decoder_format;
	//开始解析
	//DecodeCore(video_decoder_format, audio_decoder_format);
	//释放资源
	Close();
}

ImageInfo* FFMpegController::PostImageTask(SwsContext* sws_context, AVFrame* frame, int width, int height,int64_t timestamp,QImage* output)
{
	//kThreadVideoRender
	//ImageInfo* image_info = nullptr;
	//if (frame)
	//{
	//	int output_line_size[4];
	//	
	//	av_image_fill_linesizes(output_line_size, AV_PIX_FMT_ARGB, width);
	//	uint8_t* output_dst[] = { output->bits() };

	//	sws_scale(sws_context, frame->data, frame->linesize, 0, height, output_dst, output_line_size);
	//	FreeFrame(frame);
	//	//image_info = new ImageInfo(timestamp);
	//	return image_info;
	//}
	return nullptr;
}

void FFMpegController::CallFail(int code, const std::string& msg)
{
	if(!msg.empty())
	{
		//std::cout << msg << std::endl;
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

void FFMpegController::PauseAudio()
{
	//if(audio_player_core_)
	//{
	//	audio_player_core_->Pause();
	//}
}

bool FFMpegController::IsPaused()
{
	//if (audio_player_core_)
	//{
	//	return audio_player_core_->IsPaused();
	//}
	return true;
}

void FFMpegController::Seek(int64_t seek_time)
{
	//std::lock_guard<std::mutex> lock(read_packet_mutex_);
	//auto video_index = av_find_best_stream(format_context_, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);

	////transfer seek time to timebase
	//int64_t time_base_units = seek_time / 1000.0 / av_q2d(format_context_->streams[video_index]->codec->time_base);
	//int res = av_seek_frame(format_context_, video_index, seek_time, AVSEEK_FLAG_BACKWARD);
	//if(res < 0)
	//{
	//	CallFail(res,"seek time failed");
	//}
}

void FFMpegController::ResumeAudio()
{
	//if(audio_player_core_)
	//{
	//	audio_player_core_->Resume();
	//}
}

void FFMpegController::ClearCache()
{
	//image_frames_.clear();
	//if(audio_player_core_)
	//{
	//	audio_player_core_->Clear();
	//}
}

void FFMpegController::AsyncOpen()
{
	/*AVFormatContext* video_format_context = nullptr;
	AVFormatContext* audio_format_context = nullptr;
	Parse(video_format_context,true);
	Parse(audio_format_context,true);
	ViewCallback::GetInstance()->NotifyParseDone(video_format_context->duration);
	video_decoder_.Init(video_format_context);
	audio_decoder_.Init(audio_format_context);
	InitAudioPlayerCore();
	audio_player_core_->Play();
	auto video_task = [=]()
	{
		DecodeAll();
	};
	qtbase::Post2Task(kThreadDecoder, video_task);*/
}

bool FFMpegController::GetImage(ImageInfo*& image_info)
{
	//kThreadVideoRender
	//DelayFunc delay_func;
	//bool b_get = image_frames_.get_front_read_write(delay_func);
	//int normal_wait_time = frame_time_ - BASE_SLEEP_TIME;
	//if (b_get) {
	//	image_info = delay_func();
	//	int64_t audio_timestamp = 0;
	//	
	//	audio_timestamp = audio_player_core_->GetCurrentTimestamp();
	//	if (audio_timestamp < 30)
	//	{
	//		//音频可能还没开始跑，先休息一下
	//		Sleep(audio_timestamp);
	//	}
	//	else if (image_info && image_info->timestamp_ > audio_timestamp)
	//	{
	//		//视频比音频快，则让视频渲染等一会儿再渲染
	//		int64_t sleep_time = image_info->timestamp_ - audio_timestamp;
	//		sleep_time = sleep_time > MAX_ADJUST_TIME ? MAX_ADJUST_TIME + normal_wait_time : sleep_time + normal_wait_time;
	//		if (sleep_time > 0)
	//		{
	//			image_info->delay_time_ms_ = sleep_time;
	//		}
	//		return true;
	//	}
	//	else if (image_info && image_info->timestamp_ < audio_timestamp)
	//	{
	//		//视频比音频慢，则让视频渲染加快
	//		int sub_time = audio_timestamp - image_info->timestamp_;
	//		sub_time = sub_time > MAX_ADJUST_TIME ? MAX_ADJUST_TIME : sub_time;
	//		normal_wait_time -= sub_time;
	//		if (normal_wait_time > 0)
	//		{
	//			image_info->delay_time_ms_ = normal_wait_time;
	//		}
	//		return true;
	//	}
	//	else if(image_info && image_info->timestamp_ <= audio_timestamp)
	//	{
	//		image_info->delay_time_ms_ = normal_wait_time;
	//		return true;
	//	}
	//	else
	//	{
	//		return false;
	//	}
	//}
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
