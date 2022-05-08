#include <iostream>
#include <QtWidgets/QApplication>
#include "Thread/thread_pool.h"
#include "ffmpeg_qt.h"
#include "QThread"
#include "image_info/image_info.h"
static int new_cnt = 0;
static int delete_cnt = 0;
void* operator new(unsigned int size)
{
	void* ptr = (void*)malloc(size);
	//std::cout << "Alloc :" << ptr << std::endl;
	new_cnt++;
	return ptr;
}

void operator delete(void* ptr)
{
	delete_cnt++;
	//std::cout << "Delete :" << ptr << std::endl;
	free(ptr);
}

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    ThreadPool::GetInstance();
    qRegisterMetaType<ImageInfo*>("ImageInfo*");
    qRegisterMetaType<int64_t>("int64_t");
    qRegisterMetaType<std::function<void()>>("std::function<void()>");
    FFMpegQt* wid = new FFMpegQt;
    wid->show();

	a.exec();
    ThreadPool::GetInstance()->StopAll();
	return 0;
}
//
//int main()
//{
//	ThreadSafeBytesList bytes_;
//	for (int i = 0; i < 100000; i++)
//	{
//		QByteArray array_1 = "hahahahahahhaahhahahhhhhhhhhhhhhhhhhhhhhhhhhhhh";
//		//int size = array_1.size();
//		bytes_.InsertBytes(array_1, 0);
//	}
//	//bytes_.Clear();
//	int max_cnt = 30;
//	for (int i = 0; i < 100000; i++)
//	{
//		char* my_char = new char[48]();
//		std::atomic_int64_t tp = 0;
//		auto get_size = bytes_.GetBytes(max_cnt, my_char, tp);
//		my_char[47] = '\0';
//		delete[] my_char;
//		//if (get_size < max_cnt)
//		//{
//		//	break;
//		//}
//	}
//	std::cout << new_cnt <<" " << delete_cnt << std::endl;
//	return 0;
//}