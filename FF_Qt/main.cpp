#include <iostream>
#include <QtWidgets/QApplication>
#include "Thread/thread_pool.h"
#include "ffmpeg_qt.h"
#include "Audio/mq_manager.h"
#include "QThread"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    ThreadPool::GetInstance();
    FFMpegQt* wid = new FFMpegQt;
    wid->show();
	a.exec();
    ThreadPool::GetInstance()->StopAll();
	return 0;
}
//
//int main(int argc, char* argv[])
//{
//    QApplication a(argc, argv);
//
//    FFMpegQt* wid = new FFMpegQt;
//    wid->show();
//    a.exec();
//    return 0;
//}
