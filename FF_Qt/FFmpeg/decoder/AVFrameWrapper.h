#pragma once
extern "C"
{
#include <libavutil/frame.h>
}

enum FreeDataType
{
	NO_NEED_TO_FREE,
	FREE_DATA_RAW,
	FREE_DATA_AV,
};

class AVFrameWrapper
{
public:
	AVFrameWrapper(AVFrame* frame)
	{
		frame_ = frame;
		free_type_ = NO_NEED_TO_FREE;
	}
	AVFrameWrapper()
	{
		frame_ = av_frame_alloc();
		free_type_ = NO_NEED_TO_FREE;
	}
	~AVFrameWrapper()
	{
		if (frame_) 
		{
			if(free_type_ == FREE_DATA_RAW)
			{
				for (int i = 0; i < 8; i++) 
				{
					if (frame_->data[i] != NULL) 
					{
						std::unique_ptr<unsigned char*> ptr = std::make_unique<unsigned char*>(std::move(frame_->data[i]));
						frame_->data[i] = NULL;
					}
				}
			}
			else if (free_type_ == FREE_DATA_AV)
			{
				if (frame_->data[0])
				{
					av_free(frame_->data[0]);
				}
			}
			av_frame_unref(frame_);
			av_frame_free(&frame_);
		}
	}
	void SetFreeType(FreeDataType free_type)
	{
		free_type_ = free_type;
	}

	AVFrame* Frame()
	{
		return frame_;
	};

private:
	AVFrame* frame_;
	FreeDataType free_type_;
};
