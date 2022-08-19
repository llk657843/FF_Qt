#pragma once
#include "QWidget"
namespace Ui
{
	class DeviceInfoFormUI;
}
class DeviceDetect : public QWidget
{
public:
	DeviceDetect(QWidget* wid = 0);
	~DeviceDetect();

private:
	void OnModifyUI();

private:
	Ui::DeviceInfoFormUI* ui;
};