#pragma once
#include "QWidget"
#include "functional"
namespace Ui 
{ 
	class RecordSettingFormUI;
};
class RecordSettingForm : public QWidget
{
	Q_OBJECT
public:
	RecordSettingForm(QWidget* parent = 0);
	~RecordSettingForm();

private:
	void OnModifyUI();

private:
	void SlotStartClicked();
	void SlotFilePositionClicked();

private:
	Ui::RecordSettingFormUI* ui;
};