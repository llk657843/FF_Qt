#pragma once
#include <functional>
#include <qaudio.h>
#include <QObject>

#include "../base_util/singleton.h"
class ImageInfo;
using AudioStateCallback = std::function<void(QAudio::State)>;
//unused
using OpenDoneCallback = std::function<void()>;
using ImageInfoCallback = std::function<void(ImageInfo*)>;
using TimeCallback = std::function<void(int64_t timestamp)>;
using ParseDoneCallback = std::function<void(int64_t duration)>;
using RecordStateUpdateCallback = std::function<void(bool b_run)>;

class ViewCallback : public QObject
{
	Q_OBJECT
public:
	ViewCallback();
	~ViewCallback();
	SINGLETON_DEFINE(ViewCallback);
	//以观察者模式监听音频状态
	void RegAudioStateCallback(std::shared_ptr<AudioStateCallback>);
	void NotifyAudioStateCallback(QAudio::State);

	void RegImageInfoCallback(ImageInfoCallback image_info_cb);
	/**
	 * \brief 会将消息转到主线程使用
	 */
	void NotifyImageInfoCallback(ImageInfo*);


	void RegTimeCallback(TimeCallback time_cb);
	void NotifyTimeCallback(int64_t timestamp);

	void RegParseDoneCallback(ParseDoneCallback);
	void NotifyParseDone(int64_t duration);

	void RegRecordStateUpdateCallback(RecordStateUpdateCallback);
	void NotifyRecordStateUpdate(bool b_run);

	void Clear();

signals:
	void SignalImageInfo(ImageInfo*);
	void SignalTimeUpdate(int64_t);
	void SignalRecordStateUpdate(bool);

private slots:
	void SlotImageInfo(ImageInfo*);
	void SlotTimeUpdate(int64_t);
	void SlotRecordStateUpdate(bool);

private:
	std::list<std::weak_ptr<AudioStateCallback>> audio_start_callbacks_;
	ImageInfoCallback image_info_callback_;
	TimeCallback time_cb_;
	int64_t last_cb_time_;
	ParseDoneCallback parse_done_callback_;
	RecordStateUpdateCallback record_state_update_callback_;
};