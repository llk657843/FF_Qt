#include "audio_filter.h"
#include "string"
#include <iostream>
extern "C" {
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"

}	

AudioFilter::AudioFilter()
{
	avfilter_register_all();
	inputs_ = nullptr;
	outputs_ = nullptr;
}

AudioFilter::~AudioFilter()
{
	FreeBuffers();
}

void AudioFilter::InitFilter()
{
	AVFilterGraph* filter_graph = avfilter_graph_alloc();
	std::string filter_descr = "amix";
	int res =avfilter_graph_parse2(filter_graph, filter_descr.c_str(), &inputs_, &outputs_);
	if (res != 0) 
	{
		char errbuf[AV_ERROR_MAX_STRING_SIZE];
		av_make_error_string(errbuf, AV_ERROR_MAX_STRING_SIZE,res);
		std::cout << "err buf" << errbuf << std::endl;
	}
}

void AudioFilter::FreeBuffers()
{
	if (inputs_) 
	{
		avfilter_inout_free(&inputs_);
	}
	if (outputs_) 
	{
		avfilter_inout_free(&outputs_);
	}
}
