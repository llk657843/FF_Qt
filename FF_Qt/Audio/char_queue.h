#pragma once
#include <iostream>
#include <string>
class MyString
{
public:
	MyString():c_str_(nullptr),invalid_pos_str_(0),c_str_len_(0){}
	MyString(char* c_str,int c_str_len)
	{
		c_str_ = c_str;
		c_str_len_ = c_str_len;
		invalid_pos_str_ = 0;
	}
	MyString(char*&&  c_str, int c_str_len)
	{
		c_str_ = std::move(c_str);
		c_str_len_ = c_str_len;
		invalid_pos_str_ = 0;
	}

	MyString& operator=(MyString& other) noexcept
	{
		if (this == &other)
		{
			return *this;
		}
		c_str_len_ = other.c_str_len_;
		this->c_str_ = new char[other.c_str_len_];
		memcpy(this->c_str_,other.c_str_,c_str_len_);
		return *this;
	}

	MyString& operator=(MyString&& other) noexcept
	{
		if(this == &other)
		{
			return *this;
		}
		c_str_len_ = other.c_str_len_;
		this->c_str_ = std::move(other.c_str_);
		return *this;
	}

	~MyString()
	{
		//delete[] c_str_;
	}

	int ValidSize()
	{
		return c_str_len_ - invalid_pos_str_;
	}

	char* GetValidStr()
	{
		return c_str_ + invalid_pos_str_;
	}



public:
	char* c_str_;
	int invalid_pos_str_;
	int c_str_len_;
};

class CharQueue
{
	//manager current ptr
public:
	CharQueue();
	~CharQueue();

	void RefreshPos(int pos)
	{
		if(node_value)
		{
			node_value->invalid_pos_str_ = pos;
		}
	}
	bool InitNodeValue(char* data,int len);

	CharQueue* InitNext();
	CharQueue* GetNextNode();
	std::shared_ptr<MyString> GetNodeValue();
	void SetNextNode(CharQueue* new_next_node);

	int GetNodeValidSize();
	char* GetNodeValidStr();

private:
	std::shared_ptr<MyString> node_value;
	CharQueue* next_node_;
};