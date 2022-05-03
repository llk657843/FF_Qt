#pragma once
enum ThreadName 
{
	kThreadVideoRender = 0,
	kThreadDecoder,	//音视频解码线程
	kThreadAudioRender,
	kThreadHTTP,              //用户api网络请求
	kThreadLog,
	kThreadMoreTask,
};
const int ThreadCnt = 6;