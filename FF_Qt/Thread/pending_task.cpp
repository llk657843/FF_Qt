#include "pending_task.h"

ThreadTaskInfo::~ThreadTaskInfo()
{
	if (task_) 
	{
		task_.reset();
	}
}

void ThreadTaskInfo::RunTask()
{
	if (task_)
	{
		(*task_)();
	}
}

