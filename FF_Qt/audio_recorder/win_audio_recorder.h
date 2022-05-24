#pragma once
#include <windows.h>
#include "functional"
#include "../base_util/singleton.h"
#include "qbytearray.h"
#define FRAGMENT_NUM 4            // ���û���������
class WinAudioRecorder 
{
public:
	WinAudioRecorder();
	~WinAudioRecorder();
	void RecordWave();
	void StopRecord();
	WAVEFORMATEX WaveInitFormat(WORD nCh, DWORD nSampleRate, WORD BitsPerSample);

private:

	HWAVEIN phwi_;
	WAVEHDR wave_hdr_[FRAGMENT_NUM];
};