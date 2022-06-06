#include "audio_filter.h"
#include "string"
#include <iostream>
#include "../FFmpeg/decoder/AVFrameWrapper.h"
extern "C" 
{
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include <libavfilter/buffersrc.h>
}	

AudioFilter::AudioFilter()
{
	avfilter_register_all();
	audio_mix_info_ = std::make_shared<AudioInfo>();
	audio_mix_info_->name_ = "amix";     // 混音用的

	audio_sink_info_ = std::make_shared<AudioInfo>();
	audio_sink_info_->name_ = "sink";    // 输出
	filter_graph_ = nullptr;
	stream_1_pts_ = 0;
	stream_2_pts_ = 0;
}

AudioFilter::~AudioFilter()
{
	FreeBuffers();
}

void AudioFilter::InitFilter()
{
	filter_graph_ = avfilter_graph_alloc();
	char args[512] = { 0 };
	const AVFilter* amix = avfilter_get_by_name("amix");    // 混音
	audio_mix_info_->filter_ctx_ = avfilter_graph_alloc_filter(filter_graph_, amix, "amix");
	snprintf(args, sizeof(args), "inputs=%d:dropout_transition=0",
		audio_input_info_.size());

	if (avfilter_init_str(audio_mix_info_->filter_ctx_, args) != 0)
	{
		printf("[AudioMixer] avfilter_init_str(amix) failed.\n");
		return ;
	}

	const AVFilter* abuffersink = avfilter_get_by_name("abuffersink");
	audio_sink_info_->filter_ctx_ = avfilter_graph_alloc_filter(filter_graph_, abuffersink, "sink");
	if (avfilter_init_str(audio_sink_info_->filter_ctx_, nullptr) != 0)
	{
		printf("[AudioMixer] avfilter_init_str(abuffersink) failed.\n");
		return ;
	}

	int res = 0;
	for(auto &iter :audio_input_info_)
	{
		const AVFilter* abuffer = avfilter_get_by_name("abuffer");
		snprintf(args, sizeof(args),
			"sample_rate=%d:sample_fmt=%s:channel_layout=0x%I64x",
			iter.second.samplerate_,
			av_get_sample_fmt_name(iter.second.format_),
			av_get_default_channel_layout(iter.second.channels_));

		printf("[AudioMixer] input(%d) args: %s\n", iter.first, args);

		iter.second.filter_ctx_ = avfilter_graph_alloc_filter(filter_graph_, abuffer, audio_output_info_->name_.c_str());

		if (avfilter_init_str(iter.second.filter_ctx_, args) != 0)
		{
			printf("[AudioMixer] avfilter_init_str(abuffer) failed.\n");
			return ;
		}
		// iter.first 是input index
		res = avfilter_link(iter.second.filter_ctx_, 0, audio_mix_info_->filter_ctx_, iter.first);
		if (res != 0)
		{
			PrintResInfo(res);
			printf("[AudioMixer] avfilter_link(abuffer(%d), amix) failed.\n", iter.first);
			return ;
		}
	}

	if (audio_output_info_ != nullptr) 
	{
		const AVFilter* aformat = avfilter_get_by_name("aformat");
		snprintf(args, sizeof(args),
			"sample_rates=%d:sample_fmts=%s:channel_layouts=0x%I64x",
			audio_output_info_->samplerate_,
			av_get_sample_fmt_name(audio_output_info_->format_),
			av_get_default_channel_layout(audio_output_info_->channels_));
		printf("[AudioMixer] output args: %s\n", args);

		audio_output_info_->filter_ctx_ = avfilter_graph_alloc_filter(filter_graph_, aformat,
			"aformat");
		if (avfilter_init_str(audio_output_info_->filter_ctx_, args) != 0)
		{
			printf("[AudioMixer] avfilter_init_str(aformat) failed. %s\n", args);
			return ;
		}


		if (avfilter_link(audio_mix_info_->filter_ctx_, 0, audio_output_info_->filter_ctx_, 0) != 0)
		{
			printf("[AudioMixer] avfilter_link(amix, abuffersink) failed.");
			return ;
		}
		if (avfilter_link(audio_output_info_->filter_ctx_, 0, audio_sink_info_->filter_ctx_, 0) != 0)
		{
			printf("[AudioMixer] avfilter_link(aformat, abuffersink) failed.\n");
			return ;
		}
	}
	if (avfilter_graph_config(filter_graph_, NULL) < 0)
	{
		printf("[AudioMixer] avfilter_graph_config() failed.\n");
		return ;
	}
}

void AudioFilter::AddAudioInput(uint32_t index, uint32_t samplerate, uint32_t channels, uint32_t bitsPerSample, const AVSampleFormat& format)
{
	if (audio_input_info_.find(index) != audio_input_info_.end()) 
	{
		return;
	}
	auto& filterInfo = audio_input_info_[index];
	// 初始化音频相关的参数
	filterInfo.samplerate_= samplerate;
	filterInfo.channels_ = channels;
	filterInfo.bits_per_sample_ = bitsPerSample;
	filterInfo.format_ = format;
	filterInfo.name_ = std::string("input") + std::to_string(index);
}

void AudioFilter::AddAudioOutput(uint32_t samplerate, uint32_t channels, uint32_t bitsPerSample, const AVSampleFormat& format)
{
	// 初始化输出相关的参数       只有一个输出
	audio_output_info_ = std::make_shared<AudioInfo>();
	audio_output_info_->samplerate_ = samplerate;
	audio_output_info_->channels_ = channels;
	audio_output_info_->bits_per_sample_ = bitsPerSample;
	audio_output_info_->format_ = format;
	audio_output_info_->name_ = "output";
}

void AudioFilter::AddFrame(uint32_t index, uint8_t* buffer, uint32_t buffer_size)
{
	std::lock_guard<std::mutex> lock(mutex_);
	auto iter = audio_input_info_.find(index);
	if(iter == audio_input_info_.end())
	{
		return;
	}
	auto& filter_info = iter->second;
	AVFrameWrapper frame_wrapper;
	AVFrame* av_frame = frame_wrapper.Frame();
	av_frame->sample_rate = filter_info.samplerate_;
	av_frame->channel_layout = av_get_default_channel_layout(filter_info.channels_);
	av_frame->format = filter_info.format_;
	av_frame->nb_samples = 1024;
	if (index == 0) 
	{
		av_frame->pts =  stream_1_pts_;
		stream_1_pts_ += av_frame->nb_samples;
	}
	else 
	{
		av_frame->pts = stream_2_pts_;
		stream_2_pts_ += av_frame->nb_samples;
	}
	av_frame_get_buffer(av_frame, 1);
	memcpy(av_frame->extended_data[0], buffer, buffer_size);

	int res = av_buffersrc_add_frame(filter_info.filter_ctx_, av_frame);
	if (res != 0) 
	{
		PrintResInfo(res);
		printf("[AudioMixer] av_buffersrc_add_frame() failed.\n");
		return ;
	}
}

std::shared_ptr<AVFrameWrapper> AudioFilter::GetFrame(bool& b_success, uint32_t max_out_size,int& out_size)
{
	std::lock_guard<std::mutex> lock(mutex_);
	std::shared_ptr<AVFrameWrapper> frame_wrapper = std::make_shared<AVFrameWrapper>();
	AVFrame* av_frame = frame_wrapper->Frame();
	int res = av_buffersink_get_frame(audio_sink_info_->filter_ctx_, av_frame);
	if (res < 0)
	{
		PrintResInfo(res);
		b_success = false;
		return NULL;
	}
	out_size = av_samples_get_buffer_size(NULL, av_frame->channels, av_frame->nb_samples, (AVSampleFormat)av_frame->format, 1);
	if(out_size > max_out_size)
	{
		PrintResInfo(res);
		b_success = false;
		return NULL;
	}
	b_success = true;
	return frame_wrapper;
}

void AudioFilter::FreeBuffers()
{
	std::lock_guard<std::mutex> locker(mutex_);
	for (auto iter : audio_input_info_)
	{
		if (iter.second.filter_ctx_ != nullptr)
		{
			avfilter_free(iter.second.filter_ctx_);
		}
	}
	audio_input_info_.clear();
	if (audio_output_info_ && audio_output_info_->filter_ctx_)
	{
		avfilter_free(audio_output_info_->filter_ctx_);
		audio_output_info_->filter_ctx_ = nullptr;
	}

	if (audio_mix_info_->filter_ctx_)
	{
		avfilter_free(audio_mix_info_->filter_ctx_);
		audio_mix_info_->filter_ctx_ = nullptr;
	}

	if (audio_sink_info_->filter_ctx_)
	{
		avfilter_free(audio_sink_info_->filter_ctx_);
		audio_sink_info_->filter_ctx_ = nullptr;
	}

	avfilter_graph_free(&filter_graph_);
}

void AudioFilter::PrintResInfo(int res)
{
	char err_msg[AV_ERROR_MAX_STRING_SIZE];
	av_make_error_string(err_msg, AV_ERROR_MAX_STRING_SIZE, res);
	std::cout << err_msg << std::endl;
}
