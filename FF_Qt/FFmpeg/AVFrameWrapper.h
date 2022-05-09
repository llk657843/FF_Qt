#pragma once
#include <libavutil/frame.h>

class AVFrameWrapper
{
public:
	AVFrameWrapper()
	{
		frame_ = av_frame_alloc();
	}
	~AVFrameWrapper()
	{
		av_frame_unref(frame_);
		av_frame_free(&frame_);
	}

	AVFrame* Frame()
	{
		return frame_;
	};

public:
	AVFrame* frame_;
};
