#pragma once
#include <memory>
#include <qaudio.h>
#include <queue>
#include <QWidget>
#include "Audio/bytes_list.h"
#include "base_util/weak_callback.h"
#include "Thread/high_ratio_time_thread.h"
#include "base_ui/base_popup_window.h"
class ImageInfo;
class RecordSettingForm;
namespace Ui
{
	class FFMpegQtFormUI;
}
class FFMpegQt : public BasePopupWindow,public SupportWeakCallback
{
	Q_OBJECT
public:
	FFMpegQt(QWidget* wid = nullptr);
	~FFMpegQt();
	virtual std::wstring GetWindowId(void) const override;
	virtual void InitWindow() override;
protected:
	bool eventFilter(QObject* watched, QEvent* event) override;


signals:
	void SignalClose();

private:
	void OnModifyUI();
	void RegisterSignals();

private:
	void SlotStartClicked();
	void SlotResume();
	void SlotPause();
	void SlotStop();
	void SlotSliderMove(int);
	void SlotPauseResume(bool);
	void SlotClose();
	void SlotOpenFile();
	void SlotScreenShot();
	void SlotStopScreenClicked();
	void SlotMixStreamClicked();

	void ShowTime(int64_t time);
	void ShowImage(ImageInfo*);
	QString GetTimeString(int64_t time_seconds);
	void RefreshSize();

	void ShowSettingForm();

	void UpdateRecordButton(bool b_run);

private:
	Ui::FFMpegQtFormUI* ui;
	int lb_width_;
	int lb_height_;
	int64_t total_time_s_;
	ThreadSafeBytesList bytes_;
	QPointer<RecordSettingForm> record_form_;
};
