#include "ffmpeg_qt.h"

#include <iostream>
#include <thread>
#include <windows.h>

#include "Thread/thread_pool_entrance.h"
#include "ui_ffmpeg_qt.h"
#include "view_callback/view_callback.h"

FFMpegQt::FFMpegQt(QWidget* wid) : QWidget(wid),ui(new Ui::FFMpegQtFormUI)
{
	ui->setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);
	ffmpeg_control_ = nullptr;
	lb_width_ = 0;
	lb_height_ = 0;
	qRegisterMetaType<ImageInfo*>("ImageInfo*");
	OnModifyUI();
}

FFMpegQt::~FFMpegQt()
{
	delete ui;
	ui = nullptr;
}

void FFMpegQt::OnModifyUI()
{
	this->setMinimumSize(1300, 780);
	ffmpeg_control_ = std::make_unique<FFMpegController>();
	ffmpeg_control_->Init("F:/周杰伦-一路向北-(国语)[Chedvd.com].avi");
	connect(ui->btn_start,&QPushButton::clicked,this,&FFMpegQt::SlotStartClicked);
	connect(this,SIGNAL(SignalImage(ImageInfo*)),this,SLOT(SlotImage(ImageInfo*)));
	connect(this,SIGNAL(SignalStartLoop()),this,SLOT(SlotStartLoop()));

	auto view_cb = ToWeakCallback([=]()
	{
		emit SignalStartLoop();
	});

	ViewCallback::GetInstance()->RegAudioStartCallback(view_cb);
}

void FFMpegQt::SlotImage(ImageInfo* info)
{
	if(!info)
	{
		return;
	}
	int64_t start_time = time_util::GetCurrentTimeMst();
	ui->lb_movie->setPixmap(QPixmap::fromImage(info->image_));
	repaint();
	delete info;
}

void FFMpegQt::SlotStartLoop()
{
	StartLoopRender();
}

void FFMpegQt::SlotStartClicked()
{
	ffmpeg_control_->AsyncOpen();
}

void FFMpegQt::StartLoopRender()
{
	auto task = ToWeakCallback([=]()
	{
		while(ffmpeg_control_)
		{
			ImageInfo* image_info = nullptr;
			while (ffmpeg_control_->GetImage(image_info) == false)
			{
				std::this_thread::yield();
			}
			emit SignalImage(image_info);
		}
	});

	qtbase::Post2Task(kThreadVideoRender,task);
}
