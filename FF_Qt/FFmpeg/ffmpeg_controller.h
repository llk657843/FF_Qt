#pragma once
#include <functional>
#include <qimage.h>
#include <string>

#include "audio_decoder.h"
#include "video_decoder.h"
#include "../image_info/image_info.h"
#include "../Thread/threadsafe_queue.h"
class SwsContext;
class AVCodecContext;
class SwrContext;

struct AudioDecoderFormat
{
	AudioDecoderFormat()
	{
		audio_stream_index_ = 0;
		swr_context_ = nullptr;
		avc_codec_context = nullptr;
		out_channel_cnt_ = 0;
	}
	int audio_stream_index_;
	AVCodecContext* avc_codec_context;	//read only;
	SwrContext* swr_context_;	//read only;
	int out_channel_cnt_;
};

class AudioPlayerCore;
class AudioWrapper;
using FailCallback = std::function<void(int code,const std::string& msg)>;
using ImageCallback = std::function<void(QImage* image)>;
using OpenDoneCallback = std::function<void()>;
using DelayFunc = std::function<ImageInfo*(void)>;

class AVFormatContext;
class AVInputFormat;
class SwsContext;
class AVFrame;
class AVPacket;
class FFMpegController
{
public:
	FFMpegController();
	~FFMpegController();
	
	void Init(const std::string& path = "");
	void RegFailCallback(FailCallback fail_cb);
	void PauseAudio();
	bool IsPaused();
	void Seek(int64_t seek_time);

	void ResumeAudio();
	void ClearCache();

	void AsyncOpen();


	bool GetImage(ImageInfo*& image);

	void RegImageCallback(ImageCallback image_cb);
	void RegOpenDoneCallback(OpenDoneCallback);

private:
	void CallFail(int code,const std::string& msg);
	void InitSdk();
	void Close();
	void CallOpenDone();
	void InitAudioPlayerCore();
	ImageInfo* PostImageTask(SwsContext*,AVFrame*,int width,int height,int64_t timestamp,QImage* image);
	void DecodeAll();

private:
	std::string path_;
	FailCallback fail_cb_;
	AVInputFormat* av_input_;
	//AVFormatContext* format_context_;
	ImageCallback image_cb_;
	OpenDoneCallback open_done_callback_;

	VideoDecoder video_decoder_;
	AudioDecoder audio_decoder_;
};
