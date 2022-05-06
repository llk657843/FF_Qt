#include "base_decoder.h"
extern "C"
{
#include <libavformat/avformat.h>
}
BaseDecoder::BaseDecoder()
{
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
