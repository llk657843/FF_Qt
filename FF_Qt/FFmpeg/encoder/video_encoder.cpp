#include "video_encoder.h"
#include "define/bytes_info.h"
#include "define/encoder_type.h"
#include "QImage"
#include "../../Thread/time_util.h"
extern "C" 
{
#include "libavcodec/avcodec.h"
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include "libavutil/fifo.h"
#include "libavformat/avformat.h"
#include "../decoder/AVFrameWrapper.h"
#include "av_packet_wrapper.h"
}
constexpr int pix_size = 1920 * 1080;
constexpr int packet_max_size = pix_size * 3 + 1;
VideoEncoder::VideoEncoder()
{
	sws_context_ = nullptr;
	b_stop_ = false;
	video_width_ = 1920;
	video_height_ = 1080;
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

void VideoEncoder::Init(const std::string& file_name)
{
	PrepareEncode(file_name);
}

void VideoEncoder::RunEncoder()
{
	int cnt = 0;
	while (!IsEnded() && cnt++ < 10000) 
	{
		std::shared_ptr<BytesInfo> info;
		bool b_get = msg_queue_.get_front_block(info);
		if (b_get) 
		{
			//ParseImageInfo(info);
			ParseBytesInfo(info);
		}
	}
	av_write_trailer(format_context_);
}

void VideoEncoder::PostImage(std::shared_ptr<BytesInfo>&& ptr)
{
	msg_queue_.push_back_non_block(ptr);
}

void VideoEncoder::Stop()
{
	b_stop_ = true;
	msg_queue_.release();
}

bool VideoEncoder::IsEnded()
{
	return b_stop_;
}

void VideoEncoder::ParseBytesInfo(const std::shared_ptr<BytesInfo>& bytes_info)
{
	std::shared_ptr<AVFrameWrapper> frame = CreateFrame(AVPixelFormat::AV_PIX_FMT_RGB24,video_width_,video_height_,(uint8_t*)bytes_info->real_bytes_);

	auto dst_frame = CreateFrame(AVPixelFormat::AV_PIX_FMT_YUV420P, video_width_, video_height_,nullptr);
	dst_frame->Frame()->pts = 0;
	AVPacketWrapper av_packet;
	av_packet.Init();
	if (NeedConvert()) 
	{
		if (!sws_context_) 
		{
			sws_context_ = sws_getContext(codec_context_->width, codec_context_->height,
				AV_PIX_FMT_RGB24,
				codec_context_->width, codec_context_->height,
				codec_context_->pix_fmt,
				SWS_BICUBLIN, NULL, NULL, NULL);
		}
		sws_scale(sws_context_, frame->Frame()->data, frame->Frame()->linesize,
			0, codec_context_->height, dst_frame->Frame()->data,dst_frame->Frame()->linesize);
	}
	dst_frame->Frame()->best_effort_timestamp = bytes_info->frame_time_;
	dst_frame->Frame()->pts = bytes_info->frame_time_;
	dst_frame->Frame()->pkt_dts = bytes_info->frame_time_;
	//// Encode frame to packet.
	int res = avcodec_send_frame(codec_context_, dst_frame->Frame());
	if (res != 0) 
	{
		return;
	}
	res = avcodec_receive_packet(codec_context_, av_packet.Get());
	if (res != 0) 
	{
		return;
	}
	
	if (codec_context_->coded_frame->pts != AV_NOPTS_VALUE)
	{
		av_packet.Get()->pts = AV_NOPTS_VALUE;
	}
	if (codec_context_->coded_frame->key_frame)
	{
		av_packet.Get()->flags |= AV_PKT_FLAG_KEY;
	}
	av_packet.Get()->stream_index = v_stream_->index;
	res = av_interleaved_write_frame(format_context_, av_packet.Get());
	if (res != 0)
	{
		std::cout << "write frame failed" << std::endl;
		return;
	}
}
void VideoEncoder::ParseImageInfo(const std::shared_ptr<BytesInfo>& bytes_info)
{
	QImage image;
	bool b_load = image.loadFromData((const uchar*)bytes_info->bytes_,pix_size);
	image.save("D:\\1.png");
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

std::shared_ptr<AVFrameWrapper> VideoEncoder::CreateFrame(const AVPixelFormat& pix_fmt, int width, int height,uint8_t* src_ptr)
{
	auto picture = std::make_shared<AVFrameWrapper>();
	
	int size = 0;

	size = avpicture_get_size(pix_fmt, width, height);
	
	if(!src_ptr)
	{
		uint8_t* picture_buf = NULL;
		picture_buf = (uint8_t*)av_malloc(size);
		av_image_fill_arrays(picture->Frame()->data,picture->Frame()->linesize,picture_buf,pix_fmt, width, height,1);
		picture_buf = NULL;
		picture->SetManualFree(false);
	}
	else 
	{
		av_image_fill_arrays(picture->Frame()->data, picture->Frame()->linesize, src_ptr, pix_fmt, width, height, 1);
		picture->SetManualFree(true);
	}
	picture->Frame()->width = video_width_;
	picture->Frame()->height = video_height_;
	picture->Frame()->format = pix_fmt;
	return picture;
}

bool VideoEncoder::NeedConvert()
{
	bool b_res = false;
	if (v_stream_ && v_stream_->codec)
	{
		b_res = (v_stream_->codec->pix_fmt != AV_PIX_FMT_RGB24);
	}
	return b_res;
}