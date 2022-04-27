#include "char_queue.h"

CharQueue::CharQueue()
{
	node_value = nullptr;
	next_node_ = nullptr;
}

CharQueue::~CharQueue()
{
	
}

bool CharQueue::InitNodeValue(char* data, int len)
{
	if(!this->node_value)
	{
		this->node_value = std::make_shared<MyString>(data, len);
		return true;
	}
	return false;
}

CharQueue* CharQueue::InitNext()
{
	if(!this->next_node_)
	{
		this->next_node_ = new CharQueue;
	}
	return this->next_node_;
}

CharQueue* CharQueue::GetNextNode()
{
	return this->next_node_;
}

std::shared_ptr<MyString> CharQueue::GetNodeValue()
{
	return node_value;
}

void CharQueue::SetNextNode(CharQueue* new_next_node)
{
	next_node_ = new_next_node;
}

int CharQueue::GetNodeValidSize()
{
	if(node_value)
	{
		return node_value->ValidSize();
	}
	return 0;
}

char* CharQueue::GetNodeValidStr()
{
	if(node_value)
	{
		return node_value->GetValidStr();
	}
	return nullptr;
}
