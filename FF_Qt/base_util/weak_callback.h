#pragma once
#include <memory>

class WeakFlag
{
};

template<typename Function>
class WeakFunction
{
public:
	WeakFunction(const Function& func,const std::weak_ptr<WeakFlag>& weak_flag) :func_ptr_(func),weak_flag_(weak_flag){}
	WeakFunction(Function&& func,const std::weak_ptr<WeakFlag>& weak_flag) : func_ptr_(std::move(func)),weak_flag_(weak_flag){};

	template<typename WeakFunc>
	WeakFunction(const WeakFunc& weak_callback)
		: weak_flag_(weak_callback.weak_flag_)
		, func_ptr_(weak_callback.func_ptr_)
	{}

	template<typename... Args>
	auto operator()(Args... args) const
	{
		if(weak_flag_.expired())
		{
			return decltype(func_ptr_(std::forward<Args...>(args)...))();
		}
		else
		{
			return func_ptr_(std::forward<Args...>(args)...);
		}
	}
	bool Expired() const
	{
		return weak_flag_.expired();
	}


public:
	mutable Function func_ptr_;
	std::weak_ptr<WeakFlag> weak_flag_;
};

class SupportWeakCallback
{
public:
	SupportWeakCallback();
	~SupportWeakCallback();
	std::weak_ptr<WeakFlag> GetWeakFlag();


	template<typename Function>
	auto ToWeakCallback(Function closure)
	{
		return WeakFunction<Function>(closure,GetWeakFlag());
	}

protected:
	std::shared_ptr<WeakFlag> shared_flag_;
};

class WeakCallbackFlag : public SupportWeakCallback
{
public:
	WeakCallbackFlag(){}
	~WeakCallbackFlag(){}

	void Cancel()
	{
		std::atomic_store(&shared_flag_, std::shared_ptr<WeakFlag>());
	}

	bool HasUsed()
	{
		return shared_flag_.use_count() > 1;
	}


};