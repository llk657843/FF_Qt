#pragma once
#include "QWidget"


namespace base_window
{
	int GetPopupWinHeight();
	int GetWebPopupWinHeight();
	void ShowCenterWindow(QWidget* wid, QWidget* parent_wid = nullptr);
}