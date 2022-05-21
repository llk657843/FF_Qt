#pragma once
#include <mutex>
#include <string>
extern "C"
{
#include <libavcodec/avcodec.h>
}
class AVFormatContext;
class AVFrame;
class AVCodecContext;
class AVPacket;
class AVFrameWrapper;
class BaseDecoder
{
public:
	BaseDecoder();
	virtual ~BaseDecoder();
	virtual bool Init(const std::string& path) = 0;
	virtual bool Run() = 0;

protected:
	bool PrepareDeocde(const std::string& path);
	bool ReadFrame(AVPacket*& packet);
	bool SendPacket(AVCodecContext*&, AVPacket*&);
	bool ReceiveFrame(AVCodecContext*&, std::shared_ptr<AVFrameWrapper>);

protected:
	AVFormatContext* decoder_;
	std::mutex decode_mutex_;
};