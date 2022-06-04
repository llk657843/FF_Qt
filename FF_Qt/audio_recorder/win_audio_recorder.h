#pragma once
#include <windows.h>
#include "functional"
#include "../base_util/singleton.h"
#include "qbytearray.h"
#include <mutex>
#define FRAGMENT_NUM 4            // 设置缓存区个数
using AudioDatasCallback = std::function<void(char* bytes, int byte_size)>;
using RecordCloseCallback = std::function<void()>;
class WinAudioRecorder
{
public:

	WinAudioRecorder();
	~WinAudioRecorder();
	void RecordWave(const std::string& device_id);
	void RegDataCallback(AudioDatasCallback);
	void RegRecordCloseCallback(RecordCloseCallback);
	WAVEFORMATEX WaveInitFormat(WORD nCh, DWORD nSampleRate, WORD BitsPerSample);
	void StopRecord();
	

private:
	void WinEventFilter();
	void PrivateStopRecord();

private:
	HWAVEIN phwi_;
	WAVEHDR wave_hdr_[FRAGMENT_NUM];
	DWORD thread_id_;
	AudioDatasCallback audio_data_callback_;
	RecordCloseCallback record_close_callback_;
};