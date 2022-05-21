#pragma once
#include "windows.h"
class WinScreenCap 
{
public:
	WinScreenCap();
	~WinScreenCap();
	void Init();
	PRGBTRIPLE GetDesktopScreen();
	BYTE* GetScreenBytes();

private:
	HBITMAP GetCaptureBmp();


private:
	
	HDC     hDC;
	HDC     MemDC;
	HBITMAP   hBmp;
	BITMAPINFO   bi;
	BYTE* Data;
	PRGBTRIPLE m_hdib_;
};