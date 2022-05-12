#pragma once
#include <QThread>
#include "QTimer"
#include "QPointer"
using TimeoutCallback = std::function<void()>;
class HighRatioTimeThread : public QObject
{
	Q_OBJECT
public:
	HighRatioTimeThread();
	~HighRatioTimeThread();
	void InitMediaTimer();
	void Run();
	void Stop();
	void RegTimeoutCallback(TimeoutCallback);
	void SetInterval(int64_t interval_time);
	void NotifyTimeoutCallback();


private slots:
	void SlotMediaTimeout();

private:
	QPointer<QTimer> timer_;
	TimeoutCallback timeout_callback_;
	std::once_flag once_flag_;
	QThread* thread_;
};
