#pragma once
#include <functional>
using SeekResCallback = std::function<void(int64_t seek_frame, int audio_id,bool b_success)>;
