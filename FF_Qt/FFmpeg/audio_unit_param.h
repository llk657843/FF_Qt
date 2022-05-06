#pragma once
#include <memory>
#include <qbytearray.h>

class AudioUnitParam
{
public:
	AudioUnitParam() = default;
	AudioUnitParam(QByteArray&& bytes, int64_t timestamp):bytes_(std::move(bytes)),timestamp_(timestamp)
	{
		
	};
	QByteArray bytes_;
	int64_t timestamp_;
};
