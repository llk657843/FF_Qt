#pragma once
#include <memory>
#include <mutex>
#include <qaudio.h>
#include <queue>
#include <QWidget>

#include "base_util/weak_callback.h"
#include "FFmpeg/ffmpeg_controller.h"

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

signals:
	void SignalImage(ImageInfo* image);
	

private:
	void OnModifyUI();
	void RegisterSignals();

private slots:
	void SlotImage(ImageInfo*);


private:
	void SlotStartClicked();
	void SlotResume();
	void SlotPause();
	void SlotStop();

	void StartLoopRender();
	

private:
	Ui::FFMpegQtFormUI* ui;
	int lb_width_;
	int lb_height_;
	
};
