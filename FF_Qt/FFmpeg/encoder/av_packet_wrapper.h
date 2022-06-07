#pragma once

class AVPacket;
class AVPacketWrapper
{
public:
	AVPacketWrapper();
	~AVPacketWrapper();

	void Init();
	AVPacket* Get();

private:
	AVPacket* av_packet;
};