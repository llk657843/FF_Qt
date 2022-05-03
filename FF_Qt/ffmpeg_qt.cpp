#include "ffmpeg_qt.h"
#include <iostream>
#include "Thread/thread_pool_entrance.h"
#include "ui_ffmpeg_qt.h"
#include "player_controller/player_controller.h"
#include "view_callback/view_callback.h"

FFMpegQt::FFMpegQt(QWidget* wid) : QWidget(wid),ui(new Ui::FFMpegQtFormUI)
{
	ui->setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);
	lb_width_ = 0;
	lb_height_ = 0;

	OnModifyUI();
	RegisterSignals();
}

FFMpegQt::~FFMpegQt()
{
	delete ui;
	ui = nullptr;
}

void FFMpegQt::OnModifyUI()
{
	this->setMinimumSize(1300, 780);
}

void FFMpegQt::RegisterSignals()
{
	connect(ui->btn_start, &QPushButton::clicked, this, &FFMpegQt::SlotStartClicked);
	connect(ui->btn_resume, &QPushButton::clicked, this, &FFMpegQt::SlotResume);
	connect(ui->btn_pause, &QPushButton::clicked, this, &FFMpegQt::SlotPause);
	connect(ui->btn_stop, &QPushButton::clicked, this, &FFMpegQt::SlotStop);


	auto image_cb = ToWeakCallback([=](ImageInfo* image_info)
		{
		//ui thread
			ShowImage(image_info);
		});

	ViewCallback::GetInstance()->RegImageInfoCallback(image_cb);


	auto time_cb = ToWeakCallback([=](int64_t timestamp) {
		//ui thread
		ShowTime(timestamp);
		});

	ViewCallback::GetInstance()->RegTimeCallback(time_cb);
}


void FFMpegQt::SlotStartClicked()
{
	PlayerController::GetInstance()->Open();
	PlayerController::GetInstance()->Start();
}

void FFMpegQt::SlotResume()
{
	PlayerController::GetInstance()->Resume();
}

void FFMpegQt::SlotPause()
{
	PlayerController::GetInstance()->Pause();
}

void FFMpegQt::SlotStop()
{
}

void FFMpegQt::StartLoopRender()
{

}

void FFMpegQt::ShowTime(int64_t time)
{
	int64_t sec = time * 0.001;
	ui->lb_time->setText(GetTimeString(sec));
}

void FFMpegQt::ShowImage(ImageInfo* image_info)
{
	if (!image_info)
	{
		return;
	}
	ui->lb_movie->setPixmap(QPixmap::fromImage(*image_info->image_));
	repaint();
	delete image_info->image_;
	delete image_info;
}

QString FFMpegQt::GetTimeString(int64_t time_seconds)
{
	QString res_string;
	int64_t show_sec = time_seconds % 60;
	int64_t show_min = time_seconds / 60;
	res_string = QString::number(show_min) +":"+QString::number(show_sec);
	return res_string;
}
