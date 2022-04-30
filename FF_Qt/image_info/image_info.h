#pragma once
#include <qimage.h>

class ImageInfo
{
public:
	ImageInfo() = default;
	ImageInfo(const QImage& image,int64_t timestamp)
	{
		image_ = image;
		timestamp_ = timestamp;
	}
	ImageInfo(int64_t timestamp, QImage&& image)
	{
		image_ = std::move(image);
		timestamp_ = timestamp;
	}

	ImageInfo& operator=(const ImageInfo& image_info) noexcept
	{
		if(this == &image_info)
		{
			return *this;
		}
		image_ = image_info.image_;
		timestamp_ = image_info.timestamp_;
		return *this;
	}

	ImageInfo& operator=(ImageInfo&& image_info) noexcept
	{
		if (this == &image_info)
		{
			return *this;
		}
		image_ = std::move(image_info.image_);
		timestamp_ = image_info.timestamp_;
		return *this;
	}

	~ImageInfo() {}


public:
	QImage image_;
	int64_t timestamp_;
};
