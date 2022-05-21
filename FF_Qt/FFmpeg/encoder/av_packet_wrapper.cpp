#include "av_packet_wrapper.h"
extern "C"
{
#include "libavutil/mem.h"
#include <libavformat/avformat.h>
}

AVPacketWrapper::AVPacketWrapper()
{
	av_packet = (AVPacket*)av_malloc(sizeof(AVPacket));
}

AVPacketWrapper::~AVPacketWrapper()
{
	av_packet_unref(av_packet);
	av_packet_free(&av_packet);
}

void AVPacketWrapper::Init()
{
	av_init_packet(av_packet);
}

AVPacket* AVPacketWrapper::Get()
{
	return av_packet;
}
