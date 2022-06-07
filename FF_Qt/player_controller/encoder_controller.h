#pragma once
#include "../base_util/singleton.h"
#include "memory"
#include "../Thread/high_ratio_time_thread.h"
#include "../base_util/weak_callback.h"	
#include "../audio_recorder/win_audio_recorder.h"
#include "../AudioQt/audio_qt.h"
#include "define/video_encoder_param.h"
#include "../FFmpeg/encoder/define/record_state.h"
class VideoEncoder;
class WinScreenCap;
class AudioEncoder;
class EncoderCriticalSec;
class AudioFilter;
class NativeAudioController;
class EncoderController :public QObject, public SupportWeakCallback
{
	Q_OBJECT
public:
	SINGLETON_DEFINE(EncoderController);
	EncoderController();
	~EncoderController();

	void ReadyEncode();
	void StartCatch();
	void StopCapture();
	void SetBitrate(int);
	void SetFramerate(int);
	void SetPixRate(int width,int height);
	bool SetFilePath(QString);
	RecordState GetRecordState();
	QString GetCapturePath();

signals:
	void SignalStopSuccess();
	
private:
	void InitEnocderInfo(const std::string& file_path);
	void InitScreenCap();

	void InitAudioEncoder();
	void InitVideoEncoder();
	void CaptureImage();
	void CleanAll();
	void SlotStopSuccess();
	void RegCallback();
	void InitAudioRecorder();

private:
	std::unique_ptr<VideoEncoder> video_encoder_;
	std::unique_ptr<AudioEncoder> audio_encoder_;
	std::shared_ptr<EncoderCriticalSec> encoder_info_;
	std::unique_ptr<WinScreenCap> screen_cap_;
	std::unique_ptr<HighRatioTimeThread> video_capture_thread_;
	std::unique_ptr<NativeAudioController> native_audio_controller_;
	VideoEncoderParam video_param_;
	QString file_path_;
	RecordState record_state_;
};