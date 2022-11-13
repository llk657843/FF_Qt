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

template<typename T, typename SupportGuardPtrT = SupportWeakCallback, typename = std::enable_if_t<std::is_base_of<SupportGuardPtrT, T>::value>>
class GuardPtr
{
public:
	typedef decltype(std::declval<SupportGuardPtrT>().GetWeakFlag()) WeakFlag;

public:
	inline GuardPtr() {}
	inline GuardPtr(T* ptr)
		: ptr_(ptr)
		, weak_flag_(ptr ? ptr->GetWeakFlag() : WeakFlag())
	{}
	inline ~GuardPtr() {}
	// compiler-generated copy/move ctor/assignment operators are fine!

	inline GuardPtr& operator=(T* ptr) { this->Reset(ptr); return (*this); }

	inline bool IsNull() const { return (!ptr_ || weak_flag_.expired()); }

	inline T* Get() const { return IsNull() ? nullptr : ptr_; }
	inline T* operator->() const { return Get(); }
	inline T& operator*() const { return *Get(); }
	inline operator T* () const { return Get(); }

	inline void Reset() { this->Reset(nullptr); }
	inline void Reset(T* ptr)
	{
		ptr_ = ptr;
		weak_flag_ = ptr ? ptr->GetWeakFlag() : WeakFlag();
	}

	void PerformClosureOnHostThread(const std::function<void()>& closure) const
	{
		assert(closure);

		emit GuardSingleton::GetInstance()->SignalSendFunction(closure);
	}

	void TryToPerformClosureOnHostThread(const std::function<void()>& closure) const
	{
		assert(closure);
		GuardPtr guard(*this);
		emit GuardSingleton::GetInstance()->SignalSendFunction([=] {
			if (!guard.IsNull()) // guard is not null, meaning the ptr_ guarded is still valid, then we can do something on it: ptr_->XXX, *ptr_ etc.
			{
				closure();
			}
		});
	}

private:
	T* ptr_{ nullptr };
	WeakFlag weak_flag_;
};
