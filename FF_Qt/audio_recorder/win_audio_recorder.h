#pragma once
#include <windows.h>
#include "functional"
#include "../base_util/singleton.h"
#include "qbytearray.h"
#include <mutex>
#include "vector"
#define FRAGMENT_NUM 4            // 设置缓存区个数

struct AudioDevice 
{
	AudioDevice() 
	{
		device_id_ = 0;
	}
	std::wstring device_name_;
	int64_t device_id_;
};

using AudioDatasCallback = std::function<void(char* bytes, int byte_size)>;
using RecordCloseCallback = std::function<void()>;
class WinAudioRecorder
{
public:

	WinAudioRecorder();
	~WinAudioRecorder();
	void RecordWave(int default_device_id = -1);
	void RegDataCallback(AudioDatasCallback);
	void RegRecordCloseCallback(RecordCloseCallback);
	WAVEFORMATEX WaveInitFormat(WORD nCh, DWORD nSampleRate, WORD BitsPerSample);
	void StopRecord();
	std::vector<AudioDevice> GetAudioList();

private:
	void WinEventFilter();
	void PrivateStopRecord();

private:
	HWAVEIN phwi_;
	WAVEHDR wave_hdr_[FRAGMENT_NUM];
	DWORD thread_id_;
	AudioDatasCallback audio_data_callback_;
	RecordCloseCallback record_close_callback_;
	int device_id_;
};