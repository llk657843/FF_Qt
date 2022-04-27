#pragma once
#include <shared_mutex>

#include "char_queue.h"
#include "../base_util/singleton.h"
class MqManager
{
public:
	SINGLETON_DEFINE(MqManager);
	MqManager();
	~MqManager();
	void WriteData(char* data, int len);
	std::shared_ptr<MyString> ReadData(int len);
	bool IsEmpty() const;
	int GetDataSizes() const;

private:
	void Clear();

private:
	CharQueue* head_node_;
	CharQueue* last_node_;
	mutable std::shared_mutex shared_mutex_;
	std::atomic_int data_sizes_;
};