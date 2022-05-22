#include "win_screen_cap.h"
#include <QApplication>
#include "QDesktopWidget"
WinScreenCap::WinScreenCap()
{
	m_hdib_ = NULL;
}

WinScreenCap::~WinScreenCap()
{
	ReleaseDC(NULL, hDC);
	DeleteDC(MemDC);
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
	
	m_hdib_ = (PRGBTRIPLE)malloc(bi.bmiHeader.biWidth * bi.bmiHeader.biHeight * 3);//24Î»Í¼Ïñ´óÐ¡
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
