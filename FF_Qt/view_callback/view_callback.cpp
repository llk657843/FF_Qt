#include "view_callback.h"
#include "../player_controller/player_controller.h"
#include "iostream"
ViewCallback::ViewCallback()
{
	last_cb_time_ = 0;
	image_info_callback_ = nullptr;
	time_cb_ = nullptr;
	parse_done_callback_ = nullptr;
	recorder_close_callback_ = nullptr;
	record_state_update_callback_ = nullptr;
	connect(this,SIGNAL(SignalImageInfo(ImageInfo*)),this,SLOT(SlotImageInfo(ImageInfo*)));
	connect(this,SIGNAL(SignalTimeUpdate(int64_t)),this,SLOT(SlotTimeUpdate(int64_t)));
	connect(this,SIGNAL(SignalRecorderClose()),this,SLOT(SlotRecorderClose()));
	connect(this,SIGNAL(SignalConvertData(uint8_t*, uint8_t*, uint8_t*, int, int)),this,SLOT(SlotConvertData(uint8_t*, uint8_t*, uint8_t*, int, int)));
}

ViewCallback::~ViewCallback()
{

}

void ViewCallback::RegAudioStateCallback(std::shared_ptr<AudioStateCallback> cb)
{
	audio_start_callbacks_.push_back(cb);
}

void ViewCallback::NotifyAudioStateCallback(QAudio::State state)
{
	for (auto it = audio_start_callbacks_.begin();it!=audio_start_callbacks_.end();) 
	{
		auto res_ptr = (*it).lock();
		if (res_ptr)
		{
			(*res_ptr)(state);
			it++;
		}
		else
		{
			it = audio_start_callbacks_.erase(it);
		}
	}
}

void ViewCallback::RegImageInfoCallback(ImageInfoCallback image_info_cb)
{
	image_info_callback_ = image_info_cb;
}

void ViewCallback::NotifyImageInfoCallback(ImageInfo* image_info)
{
	emit SignalImageInfo(image_info);
}

void ViewCallback::RegTimeCallback(TimeCallback time_cb)
{
	time_cb_ = time_cb;
}

void ViewCallback::NotifyTimeCallback(int64_t timestamp)
{
	if (last_cb_time_ != 0)
	{
		if ((int64_t)(timestamp * 0.001) == (int64_t)(last_cb_time_ * 0.001))
		{
			return;
		}
	}
	last_cb_time_ = timestamp;
	emit SignalTimeUpdate(timestamp);
}

void ViewCallback::RegParseDoneCallback(ParseDoneCallback parse_done)
{
	parse_done_callback_ = parse_done;
}

void ViewCallback::NotifyParseDone(int64_t duration)
{
	if(parse_done_callback_)
	{
		parse_done_callback_(duration);
	}
}

void ViewCallback::RegRecordStateUpdateCallback(RecordStateUpdateCallback cb)
{
	record_state_update_callback_ = cb;
}

void ViewCallback::NotifyRecordStateUpdate(int run_state)
{
	if (record_state_update_callback_)
	{
		record_state_update_callback_(run_state);
	}
}

void ViewCallback::Clear()
{
	parse_done_callback_ = nullptr;
	image_info_callback_ = nullptr;
	time_cb_ = nullptr;
}

void ViewCallback::RegRecorderCloseCallback(RecorderCloseCallback cb)
{
	recorder_close_callback_ = cb;
}

void ViewCallback::NotifyRecorderCloseCallback()
{
	emit SignalRecorderClose();
}

void ViewCallback::RegConvertDataCb(ConvertDataCallback cb)
{
	convert_data_callback_ = cb;
}

void ViewCallback::ConvertData(uint8_t** data, int w, int h, int data_1, int data_2)
{
	unsigned char* y = nullptr;
	unsigned char* u = nullptr;
	unsigned char* v = nullptr;
	if (y == nullptr) 
	{
		y = new unsigned char[w * h];
	}
	if (u == nullptr) 
	{
		u = new unsigned char[w * h / 4];
	}
	if (v == nullptr) 
	{
		v = new unsigned char[w * h / 4];
	}

	int l1 = data_1;
	int l2 = data_2;
	int l3 = data_2;
	for (int i = 0; i < h; i++)
	{
		memcpy(y + w * i, data[0] + l1 * i, sizeof(unsigned char) * w);
	}
	for (int i = 0; i < h / 2; i++)
	{
		memcpy(u + w / 2 * i, data[1] + l2 * i, sizeof(unsigned char) * w / 2);
		memcpy(v + w / 2 * i, data[2] + l3 * i, sizeof(unsigned char) * w / 2);
	}

	emit SignalConvertData(y, u, v, w, h);
}

void ViewCallback::ConvertRGBData(uint8_t** data, int w, int h, int data_1, int data_2)
{
	//emit SignalConvertData(data[0],data[1],data[2],w,h);
	emit SignalConvertData(data[0], nullptr, nullptr, w, h);
}

void ViewCallback::SlotImageInfo(ImageInfo* image_info)
{
	if (image_info_callback_)
	{
		image_info_callback_(image_info);
	}
}

void ViewCallback::SlotTimeUpdate(int64_t timestamp)
{
	if (time_cb_)
	{
		time_cb_(timestamp);
	}
}

void ViewCallback::SlotRecorderClose()
{
	if (recorder_close_callback_)
	{
		recorder_close_callback_();
	}
}

void ViewCallback::SlotConvertData(uint8_t* y_data, uint8_t* u_data, uint8_t* v_data, int width, int height)
{
	if (convert_data_callback_) 
	{
		convert_data_callback_(y_data,u_data,v_data,width,height);
	}
}
