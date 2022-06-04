#pragma once
#include <queue>
class AVFilterInOut;
class AudioFilter 
{
public:
	AudioFilter();
	~AudioFilter();
	void InputAudioStreamA();
	void InputAudioStreamB();
	void InitFilter();
	
private:
	void FreeBuffers();

private:
	std::queue<uint8_t*> audio_stream_A_;
	std::queue<uint8_t*> audio_stream_B_;
	int current_index_;
	AVFilterInOut* inputs_;
	AVFilterInOut* outputs_;
};