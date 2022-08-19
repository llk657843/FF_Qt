#include "device_detect.h"
#include "ui_device_info_form.h"
DeviceDetect::DeviceDetect(QWidget* wid) : QWidget(wid),ui(new Ui::DeviceInfoFormUI)
{
	ui->setupUi(this);
	OnModifyUI();
}

DeviceDetect::~DeviceDetect()
{
	delete ui;
}

void DeviceDetect::OnModifyUI()
{
}
