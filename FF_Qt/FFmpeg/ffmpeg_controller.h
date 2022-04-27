#pragma once
#include <functional>
#include <qimage.h>
#include <string>
#include "../Thread/threadsafe_queue.h"
class AudioPlayerCore;
class AudioWrapper;
using FailCallback = std::function<void(int code,const std::string& msg)>;
using ImageCallback = std::function<void(QImage* image)>;
using OpenDoneCallback = std::function<void()>;
class AVFormatContext;
class AVInputFormat;

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


private:
	std::string path_;
	FailCallback fail_cb_;
	AVInputFormat* av_input_;
	AVFormatContext* format_context_;
	ImageCallback image_cb_;
	thread_safe_queue<QImage> images_;
	OpenDoneCallback open_done_callback_;
	AudioPlayerCore* audio_player_core_;
	
	int frame_;
};
