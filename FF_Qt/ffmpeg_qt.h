#pragma once
#include <memory>
#include <mutex>
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
	void SignalImage(QImage* image);

private:
	void OnModifyUI();

private slots:
	void SlotImage(QImage*);

private:
	void SlotStartClicked();
	void InsertImage(QImage* image_ptr);
	void ReadImage();
	void StartLoopRender();

private:
	Ui::FFMpegQtFormUI* ui;
	std::unique_ptr<FFMpegController> ffmpeg_control_;
	int lb_width_;
	int lb_height_;

};
