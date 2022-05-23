#include "win_audio_recorder.h"
#include "mmsystem.h"
#include "cstdio"
#include <iostream>
#define FRAGMENT_SIZE 4096        // 设置缓存区大小  
#define FRAGMENT_NUM 4            // 设置缓存区个数
WinAudioRecorder::WinAudioRecorder()
{
}

WinAudioRecorder::~WinAudioRecorder()
{
}

DWORD CALLBACK MicCallback(HWAVEIN hwavein, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	switch (uMsg)
	{
	case WIM_OPEN:
	{
		printf("\n设备已经打开...\n");
		break;
	}
	case WIM_DATA:
	{
		printf("\n缓冲区%d存满...\n", ((LPWAVEHDR)dwParam1)->dwUser);
		waveInAddBuffer(hwavein, (LPWAVEHDR)dwParam1, sizeof(WAVEHDR));
		break;
	}
	case WIM_CLOSE:
	{
		printf("\n设备已经关闭...\n");
		break;
	}
	default:
		break;
	}
	return 0;
}

void WinAudioRecorder::RecordWave()
{
	HWAVEIN phwi;

	WAVEFORMATEX pwfx = WaveInitFormat(2, 44100, 16);
	auto mmResult = waveInOpen(&phwi, WAVE_MAPPER, &pwfx, (DWORD)(MicCallback), NULL, CALLBACK_FUNCTION);//3

	if (MMSYSERR_NOERROR == mmResult)
	{
		static WAVEHDR waveHDR[FRAGMENT_NUM];
		for (int i = 0; i < FRAGMENT_NUM; i++) 
		{
			waveHDR[i].lpData = new char[FRAGMENT_SIZE];
			waveHDR[i].dwBufferLength = FRAGMENT_SIZE;
			waveHDR[i].dwBytesRecorded = 0;
			waveHDR[i].dwUser = i;
			waveHDR[i].dwFlags = 0;
			waveHDR[i].dwLoops = 1;
			waveHDR[i].lpNext = NULL;
			waveHDR[i].reserved = 0;
			waveInUnprepareHeader(phwi, &waveHDR[i], sizeof(WAVEHDR));
			//准备缓冲区
			waveInPrepareHeader(phwi, &waveHDR[i], sizeof(WAVEHDR));
			//添加buffer到音频采集
			waveInAddBuffer(phwi, &waveHDR[i], sizeof(WAVEHDR));
		}
		if (MMSYSERR_NOERROR == mmResult)
		{
			if (MMSYSERR_NOERROR == mmResult)
			{
				mmResult = waveInStart(phwi);//6
				printf("\n请求开始录音\n");
			}
		}
	}
}

WAVEFORMATEX WinAudioRecorder::WaveInitFormat(WORD nCh, DWORD nSampleRate, WORD BitsPerSample)
{
	WAVEFORMATEX m_WaveFormat;
	m_WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
	m_WaveFormat.nChannels = nCh;
	m_WaveFormat.nSamplesPerSec = nSampleRate;
	m_WaveFormat.nAvgBytesPerSec = nSampleRate * nCh * BitsPerSample / 8;
	m_WaveFormat.nBlockAlign = m_WaveFormat.nChannels * BitsPerSample / 8;
	m_WaveFormat.wBitsPerSample = BitsPerSample;
	m_WaveFormat.cbSize = 0;
	return m_WaveFormat;
}