#pragma once
enum RecordState
{
	RECORD_STATE_NONE = 0,
	RECORD_STATE_RUNNING = 1,
	RECORD_STATE_STOPPING = 2,	//正在关，还没关掉
};