#pragma once
#include "QString"
#include "vector"
#include "../base_util/singleton.h"
struct DeviceInfo
{
	QString device_name_;
	QString device_id_;
};


class DeviceController
{
public:
	SINGLETON_DEFINE(DeviceController);
	DeviceController();
	~DeviceController();

	void RefreshDevices();
	std::vector<DeviceInfo> GetCameraList();
	std::vector<DeviceInfo> GetMicrophoneList();
	std::vector<DeviceInfo> GetSpeakerList();

private:
	void InitCameraList();
	void InitMicrophoneList();
	void InitAudioList();

private:
	std::vector<DeviceInfo> camera_list_;
	std::vector<DeviceInfo> microphone_list_;
	std::vector<DeviceInfo> audio_list_;
};