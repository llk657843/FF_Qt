#include "high_ratio_time_thread.h"

#include <iostream>

HighRatioTimeThread::HighRatioTimeThread()
{
	timeout_callback_ = nullptr;
	thread_ = nullptr;
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
		timer_->setTimerType(Qt::TimerType::PreciseTimer);
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
		connect(timer_, &QTimer::timeout, this, &HighRatioTimeThread::SlotMediaTimeout, Qt::DirectConnection);
		thread_->start();
	});
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