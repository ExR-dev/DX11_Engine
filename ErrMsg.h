#pragma once

#define DEBUG_MESSAGES

#ifndef DEBUG_MESSAGES
#include <iostream>
#endif // DEBUG_MESSAGES

#include <string>


void ErrMsg(const char *msg);
void ErrMsg(const std::string &msg);

void ErrMsg(const wchar_t *msg);
void ErrMsg(const std::wstring &msg);
