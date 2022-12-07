#include "scale_manager.h"
#include <Windows.h>
#include <winuser.h>
#include "qglobal.h"
#include "QByteArray"
#include <string.h>
#include "QCoreApplication"
#include "device_detect/global_setting.h"
const float DEFAULT_DPI = 96.0;

#ifndef DPI_ENUMS_DECLARED
typedef enum PROCESS_DPI_AWARENESS
{
	PROCESS_DPI_UNAWARE = 0,
	PROCESS_SYSTEM_DPI_AWARE = 1,
	PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;
#endif
typedef BOOL(WINAPI * SETPROCESSDPIAWARE_T)(void);
typedef HRESULT(WINAPI * SETPROCESSDPIAWARENESS_T)(PROCESS_DPI_AWARENESS);


ScaleManager::ScaleManager()
{
}

ScaleManager::~ScaleManager()
{

}

bool ScaleManager::ScaleWindow()
{
	bool b_open_scale = GlobalSetting::GetInstance()->IsOpenScale();
	if (!b_open_scale || !IsWindowsRatioNeedEnabled())
	{
		return false;
	}

	bool ret = false;
#ifdef Q_OS_WIN

#ifndef DPI_ENUMS_DECLARED
	typedef enum PROCESS_DPI_AWARENESS
	{
		PROCESS_DPI_UNAWARE = 0,
		PROCESS_SYSTEM_DPI_AWARE = 1,
		PROCESS_PER_MONITOR_DPI_AWARE = 2
	} PROCESS_DPI_AWARENESS;
#endif
	typedef BOOL(WINAPI * SETPROCESSDPIAWARE_T)(void);
	typedef HRESULT(WINAPI * SETPROCESSDPIAWARENESS_T)(PROCESS_DPI_AWARENESS);

	HMODULE shcore = LoadLibraryA("Shcore.dll");
	SETPROCESSDPIAWARENESS_T SetProcessDpiAwareness = NULL;
	if (shcore) {
		//cout << "初始化shcore.dll" << endl;
		SetProcessDpiAwareness = (SETPROCESSDPIAWARENESS_T)GetProcAddress(shcore, "SetProcessDpiAwareness");
	}
	HMODULE user32 = LoadLibraryA("User32.dll");
	SETPROCESSDPIAWARE_T SetProcessDPIAware = NULL;
	if (user32) {
		//cout << "初始化User32.dll" << endl;
		SetProcessDPIAware = (SETPROCESSDPIAWARE_T)GetProcAddress(user32, "SetProcessDPIAware");
	}
	//
	//qDebug() << "SetProcessDpiAwareness---" << (*SetProcessDpiAwareness);

	if (SetProcessDpiAwareness) {
		//cout << "SetProcessDpiAwareness非零" << endl;
		ret = SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE) == S_OK;
	}
	else if (SetProcessDPIAware) {
		//cout << "SetProcessDPIAware非零" << endl;
		ret = SetProcessDPIAware() != 0;
	}
	if (user32) {
		FreeLibrary(user32);
	}
	if (shcore) {
		FreeLibrary(shcore);
	}
#endif // Q_OS_WIN

#if QT_VERSION >= QT_VERSION_CHECK(5,6,0)
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#else
	float sca = 1.0;//WinDpiScale();
	// 	float res_scale_ratio = sca * CheckResolutionRatio();
	// 	QByteArray scale_byte = QByteArray::number(res_scale_ratio, 'f', 0);
	// 	scale_ratio_ = scale_byte.toInt();
	// 	cout << "scale ratio:" << res_scale_ratio << endl;
	// 	qputenv("QT_DEVICE_PIXEL_RATIO", scale_byte);
#endif // QT_VERSION
	return ret;
}

bool ScaleManager::IsWindowsRatioNeedEnabled()
{
	DEVMODE DevMode = DEVMODE();
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &DevMode);
	int width = DevMode.dmPelsWidth;
	int height = DevMode.dmPelsHeight;

	if (width <= 1920 || height <= 1080)
	{
		return false;
	}
	return true;
}
