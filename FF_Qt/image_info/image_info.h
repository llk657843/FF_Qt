#pragma once
#include "memory"
#include "QImage"
class ImageInfo
{
public:
	ImageInfo()
	{
		image_ = nullptr;
	};
	ImageInfo(int64_t timestamp, std::shared_ptr<QImage> image)
	{
		image_ = image;
		delay_time_ms_ = 0;
		timestamp_ = timestamp;
	}


	~ImageInfo()
	{
		image_.reset();
	}


public:
	std::shared_ptr<QImage> image_;
	int64_t timestamp_;
	int64_t delay_time_ms_;
};
