#pragma once
#include "string"
#include "memory"
#include "mutex"
class AVFormatContext;
class AVStream;
class EncoderCriticalSec 
{
public:
	EncoderCriticalSec();
	~EncoderCriticalSec();
	//Call once
	bool InitFormatContext(const std::string& file_path);
	AVStream* CreateNewStream();

private:
	AVFormatContext* format_context_;
	std::mutex format_ctx_mtx_;
};