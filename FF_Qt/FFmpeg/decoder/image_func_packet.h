#pragma once
#include <functional>
class AVFrameWrapper;

extern "C"
{
#include <libavutil/frame.h>
}
class ImageInfo;
class QImage;
using DelayParseFunc = std::function<ImageInfo* (std::shared_ptr<QImage>,const std::shared_ptr<AVFrameWrapper>& frame)>;

class ImageFunc
{
public:
	ImageFunc()
	{
		image_ = nullptr;
		delay_func_ = nullptr;
		frame_ = nullptr;
	}

	~ImageFunc()
	{
		image_.reset();
		frame_.reset();
	}

	ImageFunc(std::shared_ptr<QImage> image, DelayParseFunc delay_func, const std::shared_ptr<AVFrameWrapper>& frame) : image_(std::move(image))
	{
		delay_func_ = delay_func;
		frame_ = frame;
	}

	DelayParseFunc delay_func_;
	std::shared_ptr<QImage> image_;
	std::shared_ptr<AVFrameWrapper> frame_;
};