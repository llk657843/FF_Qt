#include <iostream>
#include <QtWidgets/QApplication>
#include "Thread/thread_pool.h"
#include "ffmpeg_qt.h"
#include "QThread"
#include "image_info/image_info.h"
#include "style/qss_manager.h"
#include "player_controller/encoder_controller.h"
#include "windows.h"
#include "audio_recorder/win_audio_recorder.h"
int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    ThreadPool::GetInstance();
    qRegisterMetaType<ImageInfo*>("ImageInfo*");
    qRegisterMetaType<int64_t>("int64_t");
    qRegisterMetaType<std::function<void()>>("std::function<void()>");
    QssManager qss_manager;
    QString qss_path = a.applicationDirPath() + "/style/";
    qss_manager.SetGlobalStyle(qss_path);
    FFMpegQt* wid = new FFMpegQt;
    wid->show();
	a.exec();
    ThreadPool::GetInstance()->StopAll();

	return 0;
}