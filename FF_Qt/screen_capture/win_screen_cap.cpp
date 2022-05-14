#include "win_screen_cap.h"
#include <QApplication>
#include "QDesktopWidget"
WinScreenCap::WinScreenCap()
{
}

WinScreenCap::~WinScreenCap()
{
	ReleaseDC(NULL, hDC);
	DeleteDC(MemDC);
}

void WinScreenCap::Init()
{
	auto win_id = QApplication::desktop()->winId();
	QScreen* screen = QGuiApplication::primaryScreen();
	hDC = GetDC(NULL);
	MemDC = CreateCompatibleDC(hDC);
	int cnt = 0;
	memset(&bi, 0, sizeof(bi));
	bi.bmiHeader.biSize = sizeof(BITMAPINFO);
	bi.bmiHeader.biWidth = GetSystemMetrics(SM_CXSCREEN);
	bi.bmiHeader.biHeight = GetSystemMetrics(SM_CYSCREEN);
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 24;
	hBmp = CreateDIBSection(MemDC, &bi, DIB_RGB_COLORS, (void**)&Data, NULL, 0);
}

PRGBTRIPLE WinScreenCap::GetDesktopScreen()
{
	PRGBTRIPLE hdib = m_hdib;
	GetDIBits(MemDC, GetCaptureBmp(), 0, 1080, hdib, (LPBITMAPINFO)&bi, DIB_RGB_COLORS);
	return hdib;
}

HBITMAP WinScreenCap::GetCaptureBmp()
{
	SelectObject(MemDC, hBmp);
	BitBlt(MemDC, 0, 0, bi.bmiHeader.biWidth, bi.bmiHeader.biHeight, hDC, 0, 0, SRCCOPY);
	PRGBTRIPLE m_hdib;
	m_hdib = (PRGBTRIPLE)malloc(1920 * 1080 * 3);//24Î»Í¼Ïñ´óÐ¡
	return hBmp;
}
