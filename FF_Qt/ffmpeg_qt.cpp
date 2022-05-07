#include "ffmpeg_qt.h"
#include <iostream>
#include <windows.h>

#include "Thread/thread_pool_entrance.h"
#include "ui_ffmpeg_qt.h"
#include "player_controller/player_controller.h"
#include "view_callback/view_callback.h"
#include "QTimer"
#include "image_info/image_info.h"
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
	ui->slider->setMaximum(1000);
	ui->slider->setTickInterval(1);
}

void FFMpegQt::RegisterSignals()
{
	connect(ui->btn_start, &QPushButton::clicked, this, &FFMpegQt::SlotStartClicked);
	connect(ui->btn_resume, &QPushButton::clicked, this, &FFMpegQt::SlotResume);
	connect(ui->btn_pause, &QPushButton::clicked, this, &FFMpegQt::SlotPause);
	//connect(ui->btn_stop, &QPushButton::clicked, this, &FFMpegQt::SlotStop);
	connect(ui->slider,&USlider::sliderIsPressed,this,&FFMpegQt::SlotSliderPress);
	connect(ui->slider,&USlider::sliderIsMoved,this,&FFMpegQt::SlotSliderMove);

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

	auto parse_dur_cb = ToWeakCallback([=](int64_t timestamp)
	{
			total_time_s_ = (timestamp/1000)/1000;
	});
	ViewCallback::GetInstance()->RegParseDoneCallback(parse_dur_cb);
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

void FFMpegQt::SlotSliderPress()
{
	SlotPause();
}

void FFMpegQt::SlotSliderMove(int value)
{
	int64_t seek_time = (value / (ui->slider->tickInterval() * 1.0)) * total_time_s_;
	PlayerController::GetInstance()->SeekTime(seek_time);
}

void FFMpegQt::ShowTime(int64_t time)
{
	int64_t sec = time * 0.001;
	ui->lb_time->setText(GetTimeString(sec) +"/" + GetTimeString(total_time_s_));
	if (total_time_s_ != 0) 
	{
		//以1/1000为一个刻度
		int result = (sec / (total_time_s_ * 1.0)) * 1000.0;
		if (result != ui->slider->value())
		{
			ui->slider->setValue(result);
		}
	}
}

void FFMpegQt::ShowImage(ImageInfo* image_info)
{
	if (!image_info)
	{
		return;
	}
	ui->lb_movie->setPixmap(QPixmap::fromImage(*image_info->image_));
	update();
	delete image_info;
}

QString FFMpegQt::GetTimeString(int64_t time_seconds)
{
	QString res_string;
	int show_sec = time_seconds % 60;
	int show_min = time_seconds / 60;
	res_string = res_string.sprintf("%02d:%02d",show_min,show_sec);
	return res_string;
}