#pragma once
enum ThreadName 
{
	kThreadVideoRender = 0,
	kThreadDecoder,	//����Ƶ�����߳�
	kThreadAudioRender,
	kThreadHTTP,              //�û�api��������
	kThreadLog,
	kThreadMoreTask,
};
const int ThreadCnt = 6;