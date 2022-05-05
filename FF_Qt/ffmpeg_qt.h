#pragma once
#include <memory>
#include <mutex>
#include <qaudio.h>
#include <queue>
#include <QWidget>

#include "base_util/weak_callback.h"
#include "player_controller/high_ratio_time_thread.h"

class ImageInfo;

namespace Ui
{
	class FFMpegQtFormUI;
}
class FFMpegQt : public QWidget,public SupportWeakCallback
{
	Q_OBJECT
public:
	FFMpegQt(QWidget* wid = nullptr);
	~FFMpegQt();

private:
	void OnModifyUI();
	void RegisterSignals();

private:
	void SlotStartClicked();
	void SlotResume();
	void SlotPause();
	void SlotStop();
	void SlotSliderPress();
	void SlotSliderMove(int);

	void ShowTime(int64_t time);
	void ShowImage(ImageInfo*);
	QString GetTimeString(int64_t time_seconds);

private:
	Ui::FFMpegQtFormUI* ui;
	int lb_width_;
	int lb_height_;
	int64_t total_time_s_;
};
