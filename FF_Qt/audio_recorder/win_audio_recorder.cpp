#include "win_audio_recorder.h"
#include "mmsystem.h"
#include "cstdio"
#include <iostream>
#include "audio_data_cb.h"
#include "../view_callback/view_callback.h"
#include "../../player_controller/encoder_controller.h"
#include "../Thread/thread_pool_entrance.h"
#define FRAGMENT_SIZE 4096        // 设置缓存区大小  
#define AUDIO_MULTITHREAD_TEST
const unsigned int WM_STOP_RECORD = WM_USER + 105;
WinAudioRecorder::WinAudioRecorder()
{
	index_ = 0;
}

WinAudioRecorder::~WinAudioRecorder()
{
	for (int i = 0; i < FRAGMENT_NUM; i++) 
	{
		delete[] wave_hdr_[i].lpData;
	}
}

void WinAudioRecorder::PrivateStopRecord()
{
	std::lock_guard<std::mutex> lock(mtx_);
	MMRESULT res = waveInStop(phwi_);
	waveInReset(phwi_);
	for (int i = 0; i < 4; i++) 
	{
		res = waveInUnprepareHeader(phwi_, &wave_hdr_[i], sizeof(WAVEHDR));
	}
	res = waveInClose(phwi_);
	phwi_ = NULL;
	
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

void WinAudioRecorder::StopRecord()
{
	::PostThreadMessageA(thread_id_, WM_STOP_RECORD,NULL,NULL);
}

void WinAudioRecorder::RecordWave(const std::string& device_id)
{
	auto task = [=]() {
		WinEventFilter();
	};

	std::thread record_thread(task);
	record_thread.detach();
}

void WinAudioRecorder::WinEventFilter()
{
	thread_id_ = GetCurrentThreadId();
	WAVEFORMATEX pwfx = WaveInitFormat(2, 44100, 16);
	auto mmResult = waveInOpen(&phwi_, WAVE_MAPPER, &pwfx, GetCurrentThreadId(), (DWORD)index_, CALLBACK_THREAD);//3

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
	
	MSG msg;
	// 主消息循环:
	bool b_break = false;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		switch (msg.message)
		{
		case WIM_OPEN:
		{
			printf("\n设备已经打开...\n");
			break;
		}
		case WIM_DATA:
		{
			auto param = (LPWAVEHDR)msg.lParam;
			AudioDataCallback::GetInstance()->NotifyBufferCallback(param->lpData, param->dwBufferLength);
			waveInAddBuffer(phwi_, (LPWAVEHDR)param, sizeof(WAVEHDR));
			break;
		}
		case WIM_CLOSE:
		{
			printf("\n设备已经关闭...\n");
			b_break = true;
			ViewCallback::GetInstance()->NotifyRecorderCloseCallback();
			break;
		}
		case WM_STOP_RECORD:
		{
			PrivateStopRecord();
			break;
		}
		default:
			break;
		}
		DispatchMessage(&msg);
		if (b_break) 
		{
			break;
		}
	}
}
