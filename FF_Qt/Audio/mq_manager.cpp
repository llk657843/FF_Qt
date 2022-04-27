#include "mq_manager.h"
#include "char_queue.h"
MqManager::MqManager()
{
	data_sizes_ = 0;
	head_node_ = new CharQueue;
	last_node_ = head_node_->InitNext();
}

MqManager::~MqManager()
{
	Clear();
}

void MqManager::WriteData(char* data, int len)
{
	std::unique_lock<std::shared_mutex> lock(shared_mutex_);
	data_sizes_ += len;
	last_node_->InitNodeValue(data,len);
	last_node_ = last_node_->InitNext();
}

std::shared_ptr<MyString> MqManager::ReadData(int len)
{
	if(len <= 0)
	{
		return nullptr;
	}
	std::shared_lock<std::shared_mutex> lock(shared_mutex_);
	auto my_string = std::make_shared<MyString>();
	char* new_str = new char[len];
	int i = 0;
	int cnt = 0;
	bool b_find = false;
	CharQueue* temp_node = nullptr;
	auto cur_node = head_node_->GetNextNode();
	while(cur_node != nullptr && cur_node->GetNextNode() != nullptr && len > 0)
	{
		if(!cur_node->GetNodeValue())
		{
			break;
		}
		//当前节点的value不足以满足len
		if(cur_node->GetNodeValidSize() <= len)
		{
			strncpy(new_str + cnt,  cur_node->GetNodeValidStr(), cur_node->GetNodeValidSize());
			len = len - cur_node->GetNodeValidSize();
			cnt += cur_node->GetNodeValidSize();
			if(!b_find)
			{
				b_find = true;
			}
			temp_node = cur_node;
			cur_node = cur_node->GetNextNode();
			if(temp_node)
			{
				delete temp_node;
				temp_node = nullptr;
			}
		}
		else if(cur_node->GetNodeValidSize() > len)
		{
			char* valid_str	= cur_node->GetNodeValidStr();
			strncpy(new_str + cnt,cur_node->GetNodeValidStr(),len);
			if (!b_find)
			{
				b_find = true;
			}
			cur_node->RefreshPos(len);
			cnt += len;
			len = 0;
			break;
		}
	}
	head_node_->SetNextNode(cur_node);
	if (b_find) 
	{
		my_string->c_str_ = new_str;
		my_string->c_str_len_ = cnt;
		data_sizes_ -= cnt;
		return std::move(my_string);
	}
	else
	{
		return nullptr;
	}
}

bool MqManager::IsEmpty() const
{
	std::shared_lock<std::shared_mutex> lock(shared_mutex_);
	bool b_empty = false;
	if(head_node_->GetNextNode() == last_node_)
	{
		return last_node_->GetNodeValidSize();
	}
	return false;
}

int MqManager::GetDataSizes() const
{
	return data_sizes_;
}

void MqManager::Clear()
{
	CharQueue* cur_queue = nullptr;
	CharQueue* temp_queue = nullptr;
	if(head_node_)
	{
		cur_queue = head_node_->GetNextNode();
	}
	while(!cur_queue)
	{
		temp_queue = cur_queue;
		cur_queue = cur_queue->GetNextNode();
		if (temp_queue) 
		{
			delete temp_queue;
		}
	}
	if(head_node_)
	{
		delete head_node_;
	}
	head_node_ = nullptr;
	last_node_ = nullptr;
}
