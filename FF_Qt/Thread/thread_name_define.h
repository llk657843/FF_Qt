#pragma once
enum ThreadName 
{
	kThreadVideoRender = 0,
	kThreadVideoDecoder,
	kThreadAudioRender,
	kThreadAudioDecoder,
	kThreadHTTP,              //ÓÃ»§apiÍøÂçÇëÇó
	kThreadLog,
	kThreadMoreTask,
};
const int ThreadCnt = 7;