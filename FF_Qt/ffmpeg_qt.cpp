#include "ffmpeg_qt.h"
#include <iostream>
#include <windows.h>
#include "Thread/thread_pool_entrance.h"
#include "ui_ffmpeg_qt.h"
#include "player_controller/player_controller.h"
#include "view_callback/view_callback.h"
#include "QTimer"
#include "image_info/image_info.h"
#include "qfiledialog.h"
#include "player_controller/encoder_controller.h"
#include "view/record_setting_form.h"
#include <QtWidgets/qmessagebox.h>
const int TIME_BASE = 1000;	//刻度盘
FFMpegQt::FFMpegQt(QWidget* wid) : BasePopupWindow(wid),ui(new Ui::FFMpegQtFormUI)
{
	setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground);
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

std::wstring FFMpegQt::GetWindowId(void) const
{
	return std::wstring();
}

void FFMpegQt::InitWindow()
{
}

bool FFMpegQt::eventFilter(QObject* watched, QEvent* event)
{
	if(watched == ui->lb_movie)
	{
		if(event->type() == QEvent::Show || event->type() == QEvent::Resize)
		{
			RefreshSize();
		}
	}

	return QObject::eventFilter(watched, event);
}

void FFMpegQt::OnModifyUI()
{
	this->setMinimumSize(1300, 780);
	ui->slider->setMaximum(1000);
	ui->slider->setTickInterval(1);
	ui->fr_main->setObjectName("wid_bg");
	ui->lb_movie->setObjectName("wid_bg");
	
	ui->btn_pause_resume->setFixedSize(30, 30);

	ui->btn_stop->setFixedSize(30, 30);
	ui->btn_stop->setObjectName("btn_stop_1");
	ui->fr_bottom->setFixedHeight(50);
	ui->btn_pause_resume->setObjectName("btn_pause_resume");
	ui->btn_pause_resume->setCheckable(true);
	ui->btn_pause_resume->setChecked(false);
	ui->btn_pause_resume->setStyle(ui->btn_pause_resume->style());

	ui->btn_open_file->setObjectName("btn_folder");
	ui->btn_open_file->setFixedSize(30, 30);
	ui->lb_time->setObjectName("lb_text_1");
	
	ui->fr_top->setFixedHeight(35);

	ui->btn_min->setFixedSize(18,18);

	ui->btn_min->setObjectName("btn_min_1");
	ui->btn_max->setObjectName("btn_max_1");
	ui->btn_close->setObjectName("btn_close_1");
	ui->btn_fullscreen->setObjectName("btn_fullscreen_1");

	ui->btn_fullscreen->setFixedSize(30, 30);
	ui->btn_max->setFixedSize(18,18);
	ui->btn_close->setFixedSize(18,18);
	ui->btn_close->setCursor(Qt::PointingHandCursor);
	ui->btn_fullscreen->setCursor(Qt::PointingHandCursor);
	ui->btn_max->setCursor(Qt::PointingHandCursor);
	ui->btn_open_file->setCursor(Qt::PointingHandCursor);
	ui->btn_min->setCursor(Qt::PointingHandCursor);
	ui->btn_pause_resume->setCursor(Qt::PointingHandCursor);
	ui->btn_stop->setCursor(Qt::PointingHandCursor);
	this->CheckoutShadowType(SHADOW_TYPE::SHADOW_TYPE_MAIN_FORM);

	ui->btn_screen_shot->setCursor(Qt::PointingHandCursor);
	ui->btn_screen_shot->setObjectName("btn_record_state_normal");
	ui->btn_screen_shot->setFixedSize(30,30);
	ui->lb_movie->setObjectName("lb_img");
}

void FFMpegQt::RegisterSignals()
{
	connect(ui->btn_pause_resume, &QPushButton::clicked, this, &FFMpegQt::SlotPauseResume);
	connect(ui->btn_stop, &QPushButton::clicked, this, &FFMpegQt::SlotStop);
	connect(ui->slider,&USlider::valueChangedByMouse,this,&FFMpegQt::SlotSliderMove);
	connect(ui->btn_close,&QPushButton::clicked,this,&FFMpegQt::SlotClose);
	connect(ui->btn_open_file,&QPushButton::clicked,this,&FFMpegQt::SlotOpenFile);
	connect(ui->btn_screen_shot, &QPushButton::clicked, this, &FFMpegQt::SlotScreenShot);
	connect(this,&FFMpegQt::SignalClose,this,&FFMpegQt::close);
	//connect(ui->btn_stop_capture,&QPushButton::clicked,this,&FFMpegQt::SlotStopScreenClicked);
	ui->lb_movie->installEventFilter(this);
	auto image_cb = ToWeakCallback([=](ImageInfo* image_info)
		{
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
			//micro seconds -> seconds(show time)
			total_time_s_ = (timestamp/1000)/1000;
	});
	ViewCallback::GetInstance()->RegParseDoneCallback(parse_dur_cb);


	auto record_state_cb = ToWeakCallback([=](int run_state) {
		UpdateRecordButton(run_state == RecordState::RECORD_STATE_RUNNING);
		});

	ViewCallback::GetInstance()->RegRecordStateUpdateCallback(record_state_cb);
}


void FFMpegQt::SlotStartClicked()
{
	if (!PlayerController::GetInstance()->IsRunning()) 
	{
		PlayerController::GetInstance()->Open(lb_width_,lb_height_);
		PlayerController::GetInstance()->Start();
	}
}

void FFMpegQt::SlotResume()
{
	PlayerController::GetInstance()->Resume();
}

void FFMpegQt::SlotPause()
{
	EncoderController::GetInstance()->StartTestMemoryLeak();
	//PlayerController::GetInstance()->Pause();
}

void FFMpegQt::SlotStop()
{
	EncoderController::GetInstance()->EndTestMemoryLeak();
	//PlayerController::GetInstance()->Stop();
	//ViewCallback::GetInstance()->Clear();
	//ui->lb_movie->setPixmap(QPixmap());
}

void FFMpegQt::SlotSliderMove(int value)
{
	int64_t seek_time = (value / (ui->slider->tickInterval() * 1.0)) * total_time_s_;
	PlayerController::GetInstance()->SeekTime(seek_time);
}

void FFMpegQt::SlotPauseResume(bool b_checked)
{
	if (b_checked) 
	{
		SlotPause();
	}
	else 
	{
		SlotResume();
	}
}

void FFMpegQt::SlotClose()
{
	this->hide();
	SlotStop();
	auto delay_close = ToWeakCallback([=]() {
		emit SignalClose();
		});

	qtbase::Post2DelayedTask(kThreadMoreTask,delay_close,std::chrono::seconds(1));
}

void FFMpegQt::SlotOpenFile()
{
	QString name = QFileDialog::getOpenFileName();
	if (name.isEmpty() || name.trimmed().isEmpty()) 
	{
		return;
	}
	PlayerController::GetInstance()->SetPath(name.toStdString());
	SlotStartClicked();
}

void FFMpegQt::SlotScreenShot()
{
	auto record_state = EncoderController::GetInstance()->GetRecordState();
	if (record_state == RecordState::RECORD_STATE_NONE)
	{
		ShowSettingForm();
	}
	else if(record_state ==  RecordState::RECORD_STATE_STOPPING)
	{
		QMessageBox::information(NULL, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("视频合成中,请稍后再试"));
	}
	else 
	{
		SlotStopScreenClicked();
	}
}

void FFMpegQt::SlotStopScreenClicked()
{
	EncoderController::GetInstance()->StopCapture();
	//弹窗
	QMessageBox::information(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("录制已保存到目录") + EncoderController::GetInstance()->GetCapturePath());
}

void FFMpegQt::ShowTime(int64_t time)
{
	int64_t sec = time * 0.001;
	ui->lb_time->setText(GetTimeString(sec) +"/" + GetTimeString(total_time_s_));
	if (total_time_s_ != 0) 
	{
		int result = (sec / (total_time_s_ * 1.0)) * TIME_BASE;
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
	repaint();
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

void FFMpegQt::RefreshSize()
{
	if(lb_width_ != ui->lb_movie->width() || lb_height_ != ui->lb_movie->height())
	{
		lb_width_ = ui->lb_movie->width();
		lb_height_ = ui->lb_movie->height();
		PlayerController::GetInstance()->SetImageSize(lb_width_,lb_height_);
	}
}

void FFMpegQt::ShowSettingForm()
{
	if(!record_form_)
	{
		record_form_ = new RecordSettingForm();
	}
	record_form_->show();
}

void FFMpegQt::UpdateRecordButton(bool b_run)
{
	if(b_run)
	{
		ui->btn_screen_shot->setObjectName("btn_record_state_run");
	}
	else
	{
		ui->btn_screen_shot->setObjectName("btn_record_state_normal");
	}
	ui->btn_screen_shot->setStyle(ui->btn_screen_shot->style());
}
