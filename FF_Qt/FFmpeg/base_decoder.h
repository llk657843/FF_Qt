#pragma once
#include <mutex>
#include <string>
class AVFormatContext;
class BaseDecoder
{
public:
	BaseDecoder();
	virtual ~BaseDecoder();
	virtual bool Init(const std::string& path) = 0;
	virtual bool Run() = 0;

protected:
	bool PrepareDeocde(const std::string& path);

protected:
	AVFormatContext* decoder_;
	std::mutex decode_mutex_;
};