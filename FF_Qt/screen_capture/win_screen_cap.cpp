#include "win_screen_cap.h"
#include <QApplication>
#include "QDesktopWidget"
WinScreenCap::WinScreenCap()
{
	hBmp = NULL;
	hDC = NULL;
	MemDC = NULL;
	bit_data_ = nullptr;
}

WinScreenCap::~WinScreenCap()
{
	if (hDC) 
	{
		ReleaseDC(NULL, hDC);
		hDC = NULL;
	}
	if (MemDC) 
	{
		DeleteDC(MemDC);
		MemDC = NULL;
	}
	
	if (hBmp) 
	{
		DeleteObject(hBmp);
		hBmp = NULL;
	}
}

void WinScreenCap::Init()
{
	hDC = GetDC(NULL);
	MemDC = CreateCompatibleDC(hDC);
	int cnt = 0;
	memset(&bi, 0, sizeof(bi));
	bi.bmiHeader.biSize = sizeof(BITMAPINFO);
	bi.bmiHeader.biWidth = GetSystemMetrics(SM_CXSCREEN);
	bi.bmiHeader.biHeight = GetSystemMetrics(SM_CYSCREEN)*(-1);
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 24;
	hBmp = CreateDIBSection(MemDC, &bi, DIB_RGB_COLORS, (void**)&bit_data_, NULL, 0);
}

BYTE* WinScreenCap::GetScreenBytes()
{
	GetCaptureBmp();
	return bit_data_;
}

void WinScreenCap::GetCaptureBmp()
{
	SelectObject(MemDC, hBmp);
	BitBlt(MemDC, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), hDC, 0, 0, SRCCOPY);
}
