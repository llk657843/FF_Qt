#pragma once
enum ThreadName 
{
	kThreadVideoRender = 0,
	kThreadVideoDecoder,
	kThreadAudioRender,
	kThreadAudioDecoder,
	kThreadHTTP,              //�û�api��������
	kThreadLog,
	kThreadMoreTask,
};
const int ThreadCnt = 7;