#pragma once
#include <functional>
class ImageInfo;
class QImage;
using DelayParseFunc = std::function<ImageInfo* (std::unique_ptr<QImage>)>;

class ImageFunc
{
public:
	ImageFunc()
	{
		image_ = nullptr;
		delay_func_ = nullptr;
	}

	~ImageFunc()
	{
		delay_func_ = nullptr;
	}

	ImageFunc(std::shared_ptr<QImage> image, DelayParseFunc delay_func) : image_(std::move(image))
	{
		delay_func_ = delay_func;
	}

	ImageFunc& operator=(ImageFunc&& image_func) noexcept
	{
		if (this == &image_func)
		{
			return *this;
		}
		this->delay_func_ = image_func.delay_func_;
		this->image_ = std::move(image_func.image_);
		return *this;
	}

	ImageFunc& operator=(const ImageFunc& image_func) noexcept
	{
		if(this == &image_func)
		{
			return *this;
		}
		this->delay_func_ = image_func.delay_func_;
		this->image_ = image_func.image_;
		return *this;
	}

	std::weak_ptr<QImage> GetImage()
	{
		return image_;
	}

	DelayParseFunc delay_func_;
	std::shared_ptr<QImage> image_;
};