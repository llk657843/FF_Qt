#pragma once
#include <qimage.h>

class ImageInfo
{
public:
	ImageInfo() = default;
	ImageInfo(int64_t timestamp, QImage* image)
	{
		image_ = image;
		timestamp_ = timestamp;
	}


	~ImageInfo()
	{
		
	}


public:
	QImage* image_;
	int64_t timestamp_;
};
