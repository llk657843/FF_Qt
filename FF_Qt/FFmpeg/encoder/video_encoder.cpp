#include "video_encoder.h"
#include "define/bytes_info.h"
#include "define/encoder_type.h"
extern "C" 
{
#include "libavcodec/avcodec.h"
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include "libavutil/fifo.h"
#include "libavformat/avformat.h"
}
const int pix_size = 1920 * 1080;
VideoEncoder::VideoEncoder()
{
	av_packet_ = nullptr;
}

VideoEncoder::~VideoEncoder()
{
	if (av_packet_) 
	{
		av_free(av_packet_);
		av_packet_ = nullptr;
	}
}

void VideoEncoder::Init()
{
	PrepareEncode();
	av_packet_ = (AVPacket*)av_malloc(sizeof(AVPacket));
	av_init_packet(av_packet_);
	av_frame_ = av_frame_alloc();

	av_frame_->format = AV_PIX_FMT_YUV420P;
	av_frame_->width = 1920;
	av_frame_->height = 1080;
	
	sws_context_ = NULL;
	sws_context_ = sws_getCachedContext(nullptr, 1920, 1080, AV_PIX_FMT_YUV420P, //src w,h,fmt
		1920, 1080, AV_PIX_FMT_RGB24, //dst w,h,fmt
		SWS_BICUBIC, //尺寸变化算法
		NULL, NULL, NULL);
	frame_size_ = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, 1920, 1080, 1);
	out_buffer_yuv420_ = new BYTE[frame_size_];
	av_image_fill_arrays(av_frame_->data, av_frame_->linesize, out_buffer_yuv420_, AV_PIX_FMT_YUV420P, 1920, 1080, 1);

	if (sws_context_ == NULL)
	{
		return ;
	}

}

void VideoEncoder::RunEncoder()
{
	while (!IsEnded()) 
	{
		std::shared_ptr<BytesInfo> info;
		bool b_get = msg_queue_.get_front_block(info);
		if (b_get) 
		{
			ParseBytesInfo(info);
		}
		else 
		{
			break;
		}
	}
}

void VideoEncoder::PostImage(void* bytes, int64_t start_timestamp_ms)
{
	msg_queue_.push_back_non_block(std::make_shared<BytesInfo>(EncoderDataType::BIT_MAP_IMAGE_TYPE,static_cast<PRGBTRIPLE>(bytes),start_timestamp_ms));
}

bool VideoEncoder::IsEnded()
{
	return false;
}

void VideoEncoder::ParseBytesInfo(const std::shared_ptr<BytesInfo>& bytes_info)
{
	int out_ptr_ =0;
	RGB24_TO_YUV420((unsigned char*)(bytes_info->bytes_),1920,1080, out_buffer_yuv420_);
	
	int ret = avcodec_encode_video2(codec_context_, av_packet_, av_frame_, &out_ptr_);
	if (ret < 0)
	{
		//编码错误,不理会此帧  
		return;
	}

	if (out_ptr_ == 1)
	{
		av_packet_->stream_index = video_index_;
		av_packet_->pts = (bytes_info->frame_time_) / av_q2d(codec_context_->time_base) /1000.0;
		av_packet_->dts = av_packet_->pts;
		av_packet_->duration = 0;
		ret = av_write_frame(encoder_context_, av_packet_);
	}
	av_frame_unref(av_frame_);
}
bool VideoEncoder::RGB24_TO_YUV420(unsigned char* RgbBuf, int w, int h, unsigned char* yuvBuf)
{
	unsigned char* ptrY, * ptrU, * ptrV, * ptrRGB;
	memset(yuvBuf, 0, w * h * 3 / 2);
	ptrY = yuvBuf;
	ptrU = yuvBuf + w * h;
	ptrV = ptrU + (w * h * 1 / 4);
	unsigned char y, u, v, r, g, b;
	for (int j = h - 1; j >= 0; j--) 
	{
		ptrRGB = RgbBuf + w * j * 3;
		for (int i = 0; i < w; i++) 
		{

			b = *(ptrRGB++);
			g = *(ptrRGB++);
			r = *(ptrRGB++);


			y = (unsigned char)((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
			u = (unsigned char)((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
			v = (unsigned char)((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
			*(ptrY++) = ClipValue(y, 0, 255);
			if (j % 2 == 0 && i % 2 == 0) 
			{
				*(ptrU++) = ClipValue(u, 0, 255);
			}
			else 
			{
				if (i % 2 == 0) 
				{
					*(ptrV++) = ClipValue(v, 0, 255);
				}
			}
		}
	}
	return true;
}

unsigned char VideoEncoder::ClipValue(unsigned char x, unsigned char min_val, unsigned char  max_val) 
{
	if (x > max_val) {
		return max_val;
	}
	else if (x < min_val) {
		return min_val;
	}
	else {
		return x;
	}
}