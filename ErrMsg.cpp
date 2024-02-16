#include "ErrMsg.h"

#include <Windows.h>


inline std::wstring NarrowToWide(const std::string &narrow)
{
    if (narrow.empty())
        return { };

    const size_t reqLength = MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), (int)narrow.length(), nullptr, 0);

    std::wstring ret(reqLength, L'\0');

    MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), (int)narrow.length(), &ret[0], (int)ret.length());
    return ret;
}


void ErrMsg(const char *msg)
{
#ifdef DEBUG_MESSAGES
	OutputDebugString(NarrowToWide(msg).c_str());
	OutputDebugString(L"\n");
#else
    std::cerr << msg << std::endl;
#endif
}

void ErrMsg(const std::string &msg)
{
#ifdef DEBUG_MESSAGES
    OutputDebugString(NarrowToWide(msg).c_str());
    OutputDebugString(L"\n");
#else
    std::cerr << msg << std::endl;
#endif
}


void ErrMsg(const wchar_t *msg)
{
#ifdef DEBUG_MESSAGES
    OutputDebugString(msg);
    OutputDebugString(L"\n");
#else
    std::cerr << msg << std::endl;
#endif
}

void ErrMsg(const std::wstring &msg)
{
#ifdef DEBUG_MESSAGES
    OutputDebugString(msg.c_str());
    OutputDebugString(L"\n");
#else
    std::cerr << msg.c_str() << std::endl;
#endif
}