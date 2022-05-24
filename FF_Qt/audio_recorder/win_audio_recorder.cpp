#include "win_audio_recorder.h"
#include "mmsystem.h"
#include "cstdio"
#include <iostream>
#include "audio_data_cb.h"
#define FRAGMENT_SIZE 4096        // 设置缓存区大小  

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
		AudioDataCallback::GetInstance()->NotifyBufferCallback(((LPWAVEHDR)dwParam1)->lpData);
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
	WAVEFORMATEX pwfx = WaveInitFormat(2, 44100, 16);
	auto mmResult = waveInOpen(&phwi_, WAVE_MAPPER, &pwfx, (DWORD)(MicCallback), NULL, CALLBACK_FUNCTION);//3

	if (MMSYSERR_NOERROR == mmResult)
	{
		for (int i = 0; i < FRAGMENT_NUM; i++)
		{
			wave_hdr_[i].lpData = new char[FRAGMENT_SIZE];
			wave_hdr_[i].dwBufferLength = FRAGMENT_SIZE;
			wave_hdr_[i].dwBytesRecorded = 0;
			wave_hdr_[i].dwUser = i;
			wave_hdr_[i].dwFlags = 0;
			wave_hdr_[i].dwLoops = 1;
			wave_hdr_[i].lpNext = NULL;
			wave_hdr_[i].reserved = 0;
			waveInUnprepareHeader(phwi_, &wave_hdr_[i], sizeof(WAVEHDR));
			//准备缓冲区
			waveInPrepareHeader(phwi_, &wave_hdr_[i], sizeof(WAVEHDR));
			//添加buffer到音频采集
			waveInAddBuffer(phwi_, &wave_hdr_[i], sizeof(WAVEHDR));
		}
		mmResult = waveInStart(phwi_);
		if (MMSYSERR_NOERROR != mmResult)
		{
			std::cout << "Record start failed" << std::endl;
		}
	}
}

void WinAudioRecorder::StopRecord()
{
	auto res = waveInStop(phwi_);
	if (res != MMSYSERR_NOERROR) 
	{
		std::cout << "Record end failed" << std::endl;
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

