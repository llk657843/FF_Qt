#pragma once
#include <queue>
#include "memory"
#include <unordered_map>
#include "mutex"
extern "C" 
{
#include <libavformat/avformat.h>
}
class AVFilterInOut;
class AVFilterContext;
class AVFilterGraph;
class AudioFilter 
{
public:
	AudioFilter();
	~AudioFilter();
	void InitFilter();
	void AddAudioInput(uint32_t index, uint32_t samplerate, uint32_t channels, uint32_t bitsPerSample, const AVSampleFormat& format);
	void AddAudioOutput(uint32_t samplerate, uint32_t channels, uint32_t bitsPerSample, const AVSampleFormat& format);
	void AddFrame(uint32_t index, uint8_t* inBuf, uint32_t buffer_size);
	bool GetFrame(uint8_t* buffer, uint32_t max_out_size,int& get_size);

private:
	void FreeBuffers();
	void PrintResInfo(int res);
	
private:
	struct AudioInfo 
	{
		AudioInfo()
		{
			filter_ctx_ = nullptr;
		}
		uint32_t samplerate_;
		uint32_t channels_;
		uint32_t bits_per_sample_;
		AVSampleFormat format_;
		std::string name_;

		AVFilterContext* filter_ctx_;
	};

private:
	int current_index_;
	std::unordered_map<uint32_t, AudioInfo> audio_input_info_;
	std::shared_ptr<AudioInfo> audio_output_info_;
	std::shared_ptr<AudioInfo> audio_mix_info_;
	std::shared_ptr<AudioInfo> audio_sink_info_;
	std::mutex mutex_;
	AVFilterGraph* filter_graph_;
};