#include "ffmpeg_qt.h"
#include <iostream>
#include "Thread/thread_pool_entrance.h"
#include "ui_ffmpeg_qt.h"
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
	connect(this, SIGNAL(SignalImage(ImageInfo*)), this, SLOT(SlotImage(ImageInfo*)));
	connect(ui->btn_resume,&QPushButton::clicked,this,&FFMpegQt::SlotResume);
	connect(ui->btn_pause, &QPushButton::clicked,this,&FFMpegQt::SlotPause);
	connect(ui->btn_stop,&QPushButton::clicked,this,&FFMpegQt::SlotStop);
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


void FFMpegQt::SlotStartClicked()
{
	
}

void FFMpegQt::SlotResume()
{
	std::cout << "Warning,function empty!" << std::endl;
}

void FFMpegQt::SlotPause()
{
	
}

void FFMpegQt::SlotStop()
{
}

void FFMpegQt::StartLoopRender()
{

}
