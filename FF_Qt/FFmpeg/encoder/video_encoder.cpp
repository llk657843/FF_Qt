#include "video_encoder.h"
#include "define/bytes_info.h"
#include "define/encoder_type.h"
VideoEncoder::VideoEncoder()
{
}

VideoEncoder::~VideoEncoder()
{
}

void VideoEncoder::Init()
{
	PrepareEncode();
}

void VideoEncoder::RunEncoder()
{
	while (!IsEnded()) 
	{
		std::shared_ptr<BytesInfo> info;
		bool b_get = msg_queue_.get_front_read_write(info);
		if (b_get) 
		{

		}
		else 
		{
			std::this_thread::yield();
		}
	}
}

void VideoEncoder::PostImage(void* bytes, int64_t start_timestamp_ms)
{
	msg_queue_.push_back(std::make_shared<BytesInfo>(EncoderDataType::BIT_MAP_IMAGE_TYPE,static_cast<PRGBTRIPLE>(bytes),start_timestamp_ms));
}

bool VideoEncoder::IsEnded()
{
	return false;
}
