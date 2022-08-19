#pragma once
#include "functional"
#include "singleton.h"
#include "qobject.h"
#include "weak_callback.h"
using Func = std::function<void()>;
class GuardSingleton : public QObject
{
	Q_OBJECT
public:
	GuardSingleton();
	~GuardSingleton();
	SINGLETON_DEFINE(GuardSingleton);

signals:
	void SignalSendFunction(const Func&);

private:
	void SlotSendFunction(const Func&);
};


class GuardPtr
{
public:
	GuardPtr(const SupportWeakCallback& host)
	{
		weak_flag_ = host.GetWeakFlag();
	};

	void TryToPerformOnHostThread(const Func& function)
	{
		auto func = [=]() {
			if (!weak_flag_.expired()) 
			{
				function();
			}
		};
		emit GuardSingleton::GetInstance()->SignalSendFunction(func);
	}


private:
	std::weak_ptr<WeakFlag> weak_flag_;
};