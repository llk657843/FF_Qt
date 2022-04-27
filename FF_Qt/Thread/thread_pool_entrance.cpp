#include "thread_pool_entrance.h"
#include "thread_pool.h"

namespace qtbase
{
	void Post2Task(ThreadName thread_name, const ThreadTask& task)
	{
		ThreadPool::GetInstance()->Post2Task(thread_name, task);
	}


}