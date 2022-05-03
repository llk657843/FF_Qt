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
	void SignalStartLoop();
	void SignalStopLoop();

private:
	void OnModifyUI();

private slots:
	void SlotImage(ImageInfo*);
	void SlotStartLoop();
	void SlotStopLoop();

private:
	void SlotStartClicked();
	void InsertImage(QImage* image_ptr);
	void ReadImage();
	void StartLoopRender();

private:
	Ui::FFMpegQtFormUI* ui;
	int lb_width_;
	int lb_height_;
	std::unique_ptr<FFMpegController> ffmpeg_control_;
	std::shared_ptr<std::function<void(QAudio::State)>> audio_state_cb_;
	WeakCallbackFlag weak_flag_;
};
