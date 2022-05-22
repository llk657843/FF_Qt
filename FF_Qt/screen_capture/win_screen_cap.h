#pragma once
#include "windows.h"
class WinScreenCap 
{
public:
	WinScreenCap();
	~WinScreenCap();
	void Init();
	BYTE* GetScreenBytes();

private:
	void GetCaptureBmp();


private:
	
	HDC     hDC;
	HDC     MemDC;
	HBITMAP   hBmp;
	BITMAPINFO   bi;
	BYTE* bit_data_;
	PRGBTRIPLE m_hdib_;
};