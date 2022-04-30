#include "ffmpeg_qt.h"

#include <thread>
#include <windows.h>

#include "Thread/thread_pool_entrance.h"
#include "ui_ffmpeg_qt.h"

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
}

void FFMpegQt::SlotImage(ImageInfo* info)
{
	if(!info)
	{
		return;
	}
	/*if(!lb_width_)
	{
		lb_width_ = ui->lb_movie->width();
	}
	if(!lb_height_)
	{
		lb_height_ = ui->lb_movie->height();
	}*/
	
	ui->lb_movie->setPixmap(QPixmap::fromImage(info->image_));
	delete info;
	update();
}

void FFMpegQt::SlotStartClicked()
{
	ffmpeg_control_->AsyncOpen();
	StartLoopRender();
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
