#include "ffmpeg_qt.h"

#include <thread>
#include "Thread/thread_pool_entrance.h"
#include "ui_ffmpeg_qt.h"

FFMpegQt::FFMpegQt(QWidget* wid) : QWidget(wid),ui(new Ui::FFMpegQtFormUI)
{
	ui->setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);
	ffmpeg_control_ = nullptr;
	OnModifyUI();
}

FFMpegQt::~FFMpegQt()
{
	delete ui;
	ui = nullptr;
}

void FFMpegQt::OnModifyUI()
{
	this->setMinimumSize(800, 600);
	ffmpeg_control_ = std::make_unique<FFMpegController>();
	ffmpeg_control_->Init("F:/Mojito.mp3");
	connect(ui->btn_start,&QPushButton::clicked,this,&FFMpegQt::SlotStartClicked);
	connect(this,SIGNAL(SignalImage(QImage*)),this,SLOT(SlotImage(QImage*)));
	auto image_cb = ToWeakCallback([=](QImage* image)
	{
		emit SignalImage(image);
	});
	ffmpeg_control_->RegImageCallback(image_cb);
}

void FFMpegQt::SlotImage(QImage* image)
{
	ui->lb_movie->setPixmap(QPixmap::fromImage(*image));
	delete image;
	repaint();
}

void FFMpegQt::SlotStartClicked()
{
	ffmpeg_control_->AsyncOpen();
}

void FFMpegQt::StartLoopRender()
{
	auto task = ToWeakCallback([=]()
	{
		
		if(ffmpeg_control_)
		{
			QImage* image = new QImage;
			while (ffmpeg_control_->GetImage(*image) == false) {};
			emit SignalImage(image);
		}
	});

	qtbase::Post2RepeatedTask(kThreadUIHelper,task,std::chrono::milliseconds(41));
}
