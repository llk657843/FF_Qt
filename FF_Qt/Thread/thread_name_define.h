#pragma once
enum ThreadName 
{
	kThreadUIHelper = 0,
	kThreadVideoDecoder,
	kThreadAudioRender,
	kThreadAudioDecoder,
	kThreadHTTP,              //�û�api��������
	kThreadLog,
};
const int ThreadCnt = 6;