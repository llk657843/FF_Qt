#pragma once
class AVCodecContext;
class AVFormatContext;
class AVStream;
class BaseEncoder 
{
public:
	BaseEncoder();
	virtual ~BaseEncoder();

protected:
	bool PrepareEncode();

private:
	AVCodecContext* codec_context_;
	AVFormatContext* encoder_context_;
	AVStream* av_stream_;
};