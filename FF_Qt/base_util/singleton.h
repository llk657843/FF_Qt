#pragma once
#define SINGLETON_DEFINE(TYPENAME)\
static TYPENAME* GetInstance()\
{\
	static TYPENAME single_type_;\
	return &single_type_;\
}\
TYPENAME& operator=(const TYPENAME&) = delete;\
TYPENAME(const TYPENAME&) =delete;\
