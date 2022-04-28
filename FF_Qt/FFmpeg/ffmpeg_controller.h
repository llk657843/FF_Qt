#pragma once
#include <functional>
#include <qimage.h>
#include <string>

#include "../Thread/threadsafe_queue.h"
class SwsContext;
class AVCodecContext;
class SwrContext;
struct VideoDecoderFormat
{
	VideoDecoderFormat()
	{
		video_stream_index_ = 0;
		width_ = 0;
		height_ = 0;
		context_ = nullptr;
	}
	int video_stream_index_;
	SwsContext* context_;	//read only;
	int width_;
	int height_;
};

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
class AVFormatContext;
class AVInputFormat;
class SwsContext;
class AVFrame;

class FFMpegController
{
public:
	FFMpegController();
	~FFMpegController();
	
	void Init(const std::string& path = "");
	void RegFailCallback(FailCallback fail_cb);
	void Parse(bool b_internal = false);
	void AsyncOpen();
	bool GetImage(QImage& image);

	void RegImageCallback(ImageCallback image_cb);
	void RegOpenDoneCallback(OpenDoneCallback);

private:
	void CallFail(int code,const std::string& msg);
	void InitSdk();
	void Close();
	void DecodeVideo();
	void DecodeAudio();
	void CallOpenDone();
	void InitAudioPlayerCore();

	void PostImageTask(SwsContext*,AVFrame*,int width,int height);
	void FreeFrame(AVFrame* ptr);

	void DecodeAll();
	void InitVideoDecoderFormat(VideoDecoderFormat& video_decoder);
	void InitAudioDecoderFormat(AudioDecoderFormat& audio_decoder);

	void DecodeCore(VideoDecoderFormat&,AudioDecoderFormat&);


private:
	std::string path_;
	FailCallback fail_cb_;
	AVInputFormat* av_input_;
	AVFormatContext* format_context_;
	ImageCallback image_cb_;
	//thread_safe_queue<AVPacket> image_packets_;
	thread_safe_queue<QImage> images_;
	//thread_safe_queue<AVPacket> audios_;
	OpenDoneCallback open_done_callback_;
	AudioPlayerCore* audio_player_core_;
	
	int frame_;
};
