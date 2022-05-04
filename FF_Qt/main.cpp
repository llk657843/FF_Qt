#include <iostream>
#include <QtWidgets/QApplication>
#include "Thread/thread_pool.h"
#include "ffmpeg_qt.h"
#include "QThread"
#include "image_info/image_info.h"
int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    ThreadPool::GetInstance();
    qRegisterMetaType<ImageInfo*>("ImageInfo*");
    qRegisterMetaType<int64_t>("int64_t");
    qRegisterMetaType<std::function<void()>>("std::function<void()>");
    FFMpegQt* wid = new FFMpegQt;
    wid->show();
    //a.thread()->setPriority(QThread::Priority::HighestPriority);
	a.exec();
    ThreadPool::GetInstance()->StopAll();
	return 0;
}