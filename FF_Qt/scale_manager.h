#pragma once
#include "base\memory\singleton.h"
class ScaleManager
{
public:
	SINGLETON_DEFINE(ScaleManager);
	ScaleManager();
	~ScaleManager();
	bool ScaleWindow();
	bool IsWindowsRatioNeedEnabled();
};