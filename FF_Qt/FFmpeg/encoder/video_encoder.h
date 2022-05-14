#pragma once
#include "base_encoder.h"
#include <cstdint>
#include "../../Thread/threadsafe_queue.h"
#include "memory"

class BytesInfo;
class VideoEncoder : public BaseEncoder
{
public:
	VideoEncoder();
	~VideoEncoder();
	void Init();
	void RunEncoder();
	void PostImage(void* bytes,int64_t start_timestamp_ms);


private:
	bool IsEnded();

private:
	thread_safe_queue<std::shared_ptr<BytesInfo>> msg_queue_;

};