#pragma once
extern "C"
{
#include <libavutil/frame.h>
}
class AVFrameWrapper
{
public:
	AVFrameWrapper()
	{
		frame_ = av_frame_alloc();
		b_manual_ = false;
	}
	~AVFrameWrapper()
	{
		if (frame_) 
		{
			if (b_manual_) 
			{
				std::unique_ptr<unsigned char*> ptr = std::make_unique<unsigned char*>(std::move(frame_->data[0]));
				frame_->data[0] = NULL;
			}
			else 
			{
				av_free(frame_->data[0]);
			}
			av_frame_unref(frame_);
			av_frame_free(&frame_);
		}
	}
	void SetManualFree(bool b_manual)
	{
		b_manual_ = b_manual;
	}

	AVFrame* Frame()
	{
		return frame_;
	};

private:
	AVFrame* frame_;
	bool b_manual_ = false;
};
