#pragma once
enum ThreadName 
{
	kThreadVideoRender = 0,
	kThreadVideoDecoder,	//����Ƶ�����߳�
	kThreadAudioRender,
	kThreadAudioDecoder,
	kThreadHTTP,              //�û�api��������
	kThreadMoreTask,
	kThreadVideoEncoder,
	kThreadVideoCapture,
	kThreadAudioEncoder,
	kThreadAudioCapture,
};
const int ThreadCnt = 10;