#pragma once
#include <memory>
#include <mutex>
#include <qaudio.h>
#include <queue>
#include <QWidget>

#include "base_util/weak_callback.h"

class ImageInfo;

namespace Ui
{
	class FFMpegQtFormUI;
}
class FFMpegQt : public QWidget,public SupportWeakCallback
{
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

	void StartLoopRender();
	void ShowTime(int64_t time);
	void ShowImage(ImageInfo*);

	QString GetTimeString(int64_t time_seconds);

private:
	Ui::FFMpegQtFormUI* ui;
	int lb_width_;
	int lb_height_;
	int64_t all_time_;
};
