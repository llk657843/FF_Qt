#pragma once
enum ThreadName 
{
	kThreadVideoRender = 0,
	kThreadVideoDecoder,	//音视频解码线程
	kThreadAudioRender,
	kThreadAudioDecoder,
	kThreadHTTP,              //用户api网络请求
	kThreadMoreTask,
	kThreadVideoEncoder,
	kThreadVideoCapture,
	kThreadAudioEncoder,
	kThreadVideoCapture2,
	kThreadVideoCapture3,
};
const int ThreadCnt = 11;