#include "ffmpeg_qt.h"

#include <thread>
#include "Thread/thread_pool_entrance.h"
#include "ui_ffmpeg_qt.h"

FFMpegQt::FFMpegQt(QWidget* wid) : QWidget(wid),ui(new Ui::FFMpegQtFormUI)
{
	ui->setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);
	ffmpeg_control_ = nullptr;
	lb_width_ = 0;
	lb_height_ = 0;
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
	connect(this,SIGNAL(SignalImage(QImage*)),this,SLOT(SlotImage(QImage*)));
	auto image_cb = ToWeakCallback([=](QImage* image)
	{
		emit SignalImage(image);
	});
	//ui->lb_movie->setScaledContents(true);
	ffmpeg_control_->RegImageCallback(image_cb);
}

void FFMpegQt::SlotImage(QImage* image)
{
	if(!lb_width_)
	{
		lb_width_ = ui->lb_movie->width();
	}
	if(!lb_height_)
	{
		lb_height_ = ui->lb_movie->height();
	}
	
	ui->lb_movie->setPixmap(QPixmap::fromImage(*image));
	delete image;
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
		if(ffmpeg_control_)
		{
			QImage* image = new QImage;
			while (ffmpeg_control_->GetImage(*image) == false)
			{
				std::this_thread::yield();
			}
			emit SignalImage(image);
		}
	});

	qtbase::Post2RepeatedTask(kThreadVideoRender,task,std::chrono::milliseconds(40));
}
