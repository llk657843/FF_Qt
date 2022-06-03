#pragma once
#include <windows.h>
#include "functional"
#include "../base_util/singleton.h"
#include "qbytearray.h"
#include <mutex>
#define FRAGMENT_NUM 4            // 设置缓存区个数
class WinAudioRecorder
{
public:
	WinAudioRecorder();
	~WinAudioRecorder();
	void RecordWave(const std::string& device_id);
	
	WAVEFORMATEX WaveInitFormat(WORD nCh, DWORD nSampleRate, WORD BitsPerSample);
	void StopRecord();

private:

	void WinEventFilter();
	void PrivateStopRecord();
private:
	HWAVEIN phwi_;
	WAVEHDR wave_hdr_[FRAGMENT_NUM];
	std::mutex mtx_;
	int index_;
	DWORD thread_id_;
};