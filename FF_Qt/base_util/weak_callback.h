#pragma once
#include <atomic>
#include <memory>
#include <functional>
typedef std::function<void(void)> StdClosure;
class WeakFlag{};
template<typename T>
class WeakFunction
{
public:
	WeakFunction(const std::weak_ptr<WeakFlag>& weak_flag, const T& t)
		: weak_flag_(weak_flag)
		, t_(t)
	{}

	WeakFunction(const std::weak_ptr<WeakFlag>& weak_flag, T&& t)
		: weak_flag_(weak_flag)
		, t_(std::move(t))
	{}

	template<class WeakType>
	WeakFunction(const WeakType& weak_callback)
		: weak_flag_(weak_callback.weak_flag_)
		, t_(weak_callback.t_)
	{}

	template<class... Args>
	auto operator ()(Args && ... args) const
		->decltype(t_(std::forward<Args>(args)...))
	{
		if (!weak_flag_.expired()) {
			return t_(std::forward<Args>(args)...);
		}

		return decltype(t_(std::forward<Args>(args)...))();
	}

	bool Expired() const
	{
		return weak_flag_.expired();
	}

	std::weak_ptr<WeakFlag> weak_flag_;
	mutable T t_;
};

class SupportWeakCallback
{
public:
	SupportWeakCallback(){}
	virtual ~SupportWeakCallback() {};

	std::weak_ptr<WeakFlag> GetWeakFlag()
	{
		auto weak_flag = std::atomic_load(&m_weakFlag);
		if (!weak_flag)
		{
			std::shared_ptr<WeakFlag> weak_flag_new((WeakFlag*)0x01, [](WeakFlag*) {/*do nothing with the fake pointer 0x01*/});

			while (!std::atomic_compare_exchange_weak(&m_weakFlag, &weak_flag, weak_flag_new));
		}

		return m_weakFlag;
	}

	template<typename CallbackType>
	auto ToWeakCallback(const CallbackType& closure)
		->WeakFunction<CallbackType>
	{
		return WeakFunction<CallbackType>(GetWeakFlag(), closure);
	}

protected:
	std::shared_ptr<WeakFlag> m_weakFlag;
};

	//WeakCallbackFlag一般作为类成员变量使用，要继承，可使用不带Cancel()函数的SupportWeakCallback
	//这里禁止继承，主要担心误用。当使用这个类的功能，打包出多个支持weak语义的callback时，一旦错误的调用了Cancel，
	//将会取消所有callback，这种情况可能不是用户希望的。此时，应该使用多个不带Cancel函数的WeakCallbackFlag类型的成员变量，
	//每个对应一个callback，一一对应的控制每个支持weak语义的callback。
class WeakCallbackFlag final : public SupportWeakCallback
{
public:
	void Cancel()
	{
		std::atomic_store(&m_weakFlag, std::shared_ptr<WeakFlag>());
	}

	bool HasUsed()
	{
		return m_weakFlag.use_count() != 0;
	}
};