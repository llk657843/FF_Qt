#pragma once
enum ThreadName 
{
	kThreadUIHelper = 0,
	kThreadVideoDecoder,
	kThreadAudioRender,
	kThreadAudioDecoder,
	kThreadHTTP,              //ÓÃ»§apiÍøÂçÇëÇó
	kThreadLog,
};
const int ThreadCnt = 6;