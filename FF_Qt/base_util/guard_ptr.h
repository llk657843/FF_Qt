//#pragma once
//#include <type_traits>
//#include "weak_callback.h"
//#include "closure.h"
//
//using Closure = std::function<void()>;
//template<typename T>
//using IsSupportWeakBase = std::enable_if_t<std::is_base_of<SupportWeakCallback, T>::value>;
//template<typename T,typename = IsSupportWeakBase<T>>
//class GuardPtr
//{
//public:
//	GuardPtr(T* Parent)
//	{
//		QObject::connect(signal_, &QObjectWithClosureSignal::SignalClosure, this, [](const std::function<void()>& closure)
//			{
//				closure();
//			});
//		weak_ = Parent->GetWeakFlag();
//	}
//	~GuardPtr(){}
//
//	void TryPerformOnHost(Closure closure)
//	{
//		if(!weak_.expired())
//		{
//			emit signal_.SignalClosure(closure);
//		}
//	}
//
//private:
//	std::weak_ptr<WeakFlag> weak_;
//	QObjectWithClosureSignal signal_;
//};