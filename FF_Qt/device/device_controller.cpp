#include "device_controller.h"
#include "windows.h"
DeviceController::DeviceController()
{
}

DeviceController::~DeviceController()
{
}

void DeviceController::RefreshDevices()
{
	InitCameraList();
	InitAudioList();
	InitMicrophoneList();
}

std::vector<DeviceInfo> DeviceController::GetCameraList()
{
	return camera_list_;
}

std::vector<DeviceInfo> DeviceController::GetMicrophoneList()
{
	return microphone_list_;
}

std::vector<DeviceInfo> DeviceController::GetSpeakerList()
{
	return audio_list_;
}

void DeviceController::InitCameraList()
{
	
}

void DeviceController::InitMicrophoneList()
{
	microphone_list_.clear();
	UINT num_devices = waveInGetNumDevs();
	for (unsigned int device_id = 0; device_id < num_devices; device_id++)
	{
		DeviceInfo device;
		WAVEINCAPS device_caps;
		MMRESULT result = waveInGetDevCaps(UINT_PTR(device_id), &device_caps, sizeof(WAVEINCAPS));
		if (result != MMSYSERR_NOERROR) {

			continue;
		}
		device.device_name_ = QString::fromStdWString(device_caps.szPname);
		device.device_id_ = device_caps.wPid;
		microphone_list_.push_back(device);
	}
}

void DeviceController::InitAudioList()
{
	audio_list_.clear();
	UINT num_devices = waveOutGetNumDevs();
	for (unsigned int device_id = 0; device_id < num_devices; device_id++)
	{
		DeviceInfo device;
		WAVEOUTCAPS device_caps;
		MMRESULT result = waveOutGetDevCaps(UINT_PTR(device_id), &device_caps, sizeof(WAVEOUTCAPS));
		if (result != MMSYSERR_NOERROR) {

			continue;
		}
		device.device_name_ = QString::fromStdWString(device_caps.szPname);
		device.device_id_ = device_caps.wPid;
		audio_list_.push_back(device);
	}
}
