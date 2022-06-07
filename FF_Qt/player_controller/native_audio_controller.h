#pragma once
#include "memory"
#include "../base_util/weak_callback.h"
#include "functional"
#include "../FFmpeg/decoder/AVFrameWrapper.h"
#include "atomic"
class WinAudioRecorder;
class HighRatioTimeThread;
class AudioFilter;

using AVFrameDataCallback = std::function<void(AVFrame*,int buffer_size)>;
using StopRecordCallback = std::function<void()>;
class NativeAudioController : public SupportWeakCallback
{
public:
	NativeAudioController();
	~NativeAudioController();
	void StartRun();
	void RegDataCallback(AVFrameDataCallback);
	void StopRunAsync();
	void RegStopRecordCallback(StopRecordCallback);

private:
	void InitRecorder();
	void InitFilter();
	void RecordClose();
	void AddStream(int index,char* buffer,int buffer_size);
	void StartGetBufferLoop();

private:
	std::unique_ptr<WinAudioRecorder> recorder_;
	std::unique_ptr<WinAudioRecorder> mic_recorder_;
	std::unique_ptr<AudioFilter> audio_filter_;
	std::unique_ptr<HighRatioTimeThread> time_thread_;
	AVFrameDataCallback frame_data_cb_;
	StopRecordCallback stop_record_cb_;
	std::atomic_int close_stream_cnt_;
};