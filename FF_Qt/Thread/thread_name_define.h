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
	kThreadVideoCapture2,
	kThreadVideoCapture3,
};
const int ThreadCnt = 11;