#include "guard_ptr.h"


GuardSingleton::GuardSingleton()
{
	qRegisterMetaType<std::function<void()>>("std::function<void()>");
	connect(this,&GuardSingleton::SignalSendFunction,this,&GuardSingleton::SlotSendFunction);
}

GuardSingleton::~GuardSingleton()
{
}

void GuardSingleton::SlotSendFunction(const Func& function)
{
	function();
}