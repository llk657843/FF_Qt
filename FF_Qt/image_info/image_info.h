#pragma once
#include <qimage.h>

class ImageInfo
{
public:
	ImageInfo()
	{
		image_ = nullptr;
	};
	ImageInfo(int64_t timestamp, QImage* image)
	{
		image_ = image;
		delay_time_ms_ = 0;
		timestamp_ = timestamp;
	}


	~ImageInfo()
	{
		delete image_;
	}


public:
	QImage* image_;
	int64_t timestamp_;
	int64_t delay_time_ms_;
};
