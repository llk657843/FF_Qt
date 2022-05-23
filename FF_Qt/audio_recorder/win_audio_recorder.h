#pragma once
#include <windows.h>
class WinAudioRecorder 
{
public:
	WinAudioRecorder();
	~WinAudioRecorder();
	void RecordWave();
	WAVEFORMATEX WaveInitFormat(WORD nCh, DWORD nSampleRate, WORD BitsPerSample);
};