#include "weak_callback.h"

SupportWeakCallback::SupportWeakCallback()
{
	shared_flag_ = std::make_shared<WeakFlag>();
}

SupportWeakCallback::~SupportWeakCallback()
{
}

std::weak_ptr<WeakFlag> SupportWeakCallback::GetWeakFlag() const
{
	return shared_flag_;
}
