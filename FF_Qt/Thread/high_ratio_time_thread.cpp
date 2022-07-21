#include "high_ratio_time_thread.h"
#include <QElapsedTimer>
HighRatioTimeThread::HighRatioTimeThread(bool b_high_ratio)
{
	timeout_callback_ = nullptr;
	thread_ = nullptr;
	b_main_thread_ = false;
	b_high_ratio_ = b_high_ratio;
	InitMediaTimer();
}

HighRatioTimeThread::~HighRatioTimeThread()
{
	if(timeout_callback_)
	{
		timeout_callback_ = nullptr;
	}
	if (timer_) 
	{
		disconnect(timer_, &QTimer::timeout, this, &HighRatioTimeThread::SlotMediaTimeout);
	}
}

void HighRatioTimeThread::InitMediaTimer()
{
	if (!timer_)
	{
		timer_ = new QTimer();
		if (b_high_ratio_) 
		{
			timer_->setTimerType(Qt::TimerType::PreciseTimer);
		}
		else
		{
			timer_->setTimerType(Qt::TimerType::CoarseTimer);
		}
		timer_->setInterval(20);
	}
	if(!thread_)
	{
		thread_ = new QThread();
	}
}

void HighRatioTimeThread::Run()
{
	std::call_once(once_flag_,[=]()
	{
		timer_->start();
		timer_->moveToThread(thread_);
		if (b_main_thread_) 
		{
			connect(timer_, &QTimer::timeout, this, &HighRatioTimeThread::SlotMediaTimeout);
		}
		else 
		{
			connect(timer_, &QTimer::timeout, this, &HighRatioTimeThread::SlotMediaTimeout, Qt::DirectConnection);
		}
		thread_->start();
	});
}

void HighRatioTimeThread::SetConnectMainThread()
{
	b_main_thread_ = true;
}

void HighRatioTimeThread::Stop()
{
	timeout_callback_ = nullptr;
	thread_->exit();
}

void HighRatioTimeThread::RegTimeoutCallback(TimeoutCallback cb)
{
	timeout_callback_ = cb;
}

void HighRatioTimeThread::SetInterval(int64_t interval_time)
{
	if (timer_->interval() != interval_time) 
	{
		timer_->setInterval(interval_time);
	}
}

void HighRatioTimeThread::NotifyTimeoutCallback()
{
	if(timeout_callback_)
	{
		timeout_callback_();
	}
}

void HighRatioTimeThread::SlotMediaTimeout()
{
	NotifyTimeoutCallback();
}