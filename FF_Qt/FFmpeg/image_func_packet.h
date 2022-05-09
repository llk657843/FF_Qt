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

	//ImageFunc(const ImageFunc& image_func) noexcept
	//{
	//	this->delay_func_ = image_func.delay_func_;
	//	this->image_ = image_func.image_;
	//	this->frame_ = image_func.frame_;
	//}

	//ImageFunc(const ImageFunc&& image_func) noexcept
	//{
	//	this->delay_func_ = std::move(image_func.delay_func_);
	//	this->image_ = std::move(image_func.image_);
	//	this->frame_ = std::move(image_func.frame_);
	//}


	//ImageFunc& operator=(const ImageFunc& image_func) noexcept
	//{
	//	if(this == &image_func)
	//	{
	//		return *this;
	//	}
	//	this->delay_func_ = image_func.delay_func_;
	//	this->image_ = image_func.image_;
	//	this->frame_ = image_func.frame_;
	//	return *this;
	//}

	//ImageFunc& operator=(ImageFunc&& image_func) noexcept
	//{
	//	if (this == &image_func)
	//	{
	//		return *this;
	//	}
	//	this->delay_func_ = std::move(image_func.delay_func_);
	//	this->image_ = std::move(image_func.image_);
	//	this->frame_ = std::move(image_func.frame_);
	//	return *this;
	//}

	std::weak_ptr<QImage> GetImage()
	{
		return image_;
	}

	DelayParseFunc delay_func_;
	std::shared_ptr<QImage> image_;
	std::shared_ptr<AVFrameWrapper> frame_;
};