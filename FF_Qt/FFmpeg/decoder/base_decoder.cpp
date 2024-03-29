#include "base_decoder.h"
#include "AVFrameWrapper.h"
extern "C"
{
#include <libavformat/avformat.h>
}
BaseDecoder::BaseDecoder()
{
	decoder_ = nullptr;
}

BaseDecoder::~BaseDecoder()
{
}

bool BaseDecoder::PrepareDeocde(const std::string& path)
{
	if(decoder_)
	{
		return false;
	}

	if (path.empty())
	{
		return false;
	}

	AVDictionary* av_dic = nullptr;
	av_dict_set(&av_dic,"rtsp_transport","tcp",0);

	AVInputFormat* av_input = nullptr;
	int res = avformat_open_input(&decoder_, path.c_str(), av_input, NULL);
	if (res != 0)
	{
		return false;
	}

	res = avformat_find_stream_info(decoder_, NULL);
	if (res != 0)
	{
		return false;
	}

	av_dump_format(decoder_, NULL, path.c_str(), 0);
	av_free(av_input);
	return true;
}

bool BaseDecoder::ReadFrame(AVPacket*& packet)
{
	std::lock_guard<std::mutex> lock(decode_mutex_);
	return av_read_frame(decoder_, packet) >= 0;
}

bool BaseDecoder::SendPacket(AVCodecContext*& codec_context,AVPacket*& packet)
{
	std::lock_guard<std::mutex> lock(decode_mutex_);
	return avcodec_send_packet(codec_context, packet) != 0;
}

bool BaseDecoder::ReceiveFrame(AVCodecContext*& codec_context, std::shared_ptr<AVFrameWrapper> frame)
{
	std::lock_guard<std::mutex> lock(decode_mutex_);
	return avcodec_receive_frame(codec_context, frame->Frame()) != 0;
}