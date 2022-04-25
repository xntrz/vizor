#pragma once

#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objidl.h>
#include <Shlwapi.h>
#include <windowsx.h>
#include <CommCtrl.h>
#include <commdlg.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <Uxtheme.h>
#include <shellapi.h>
#include <tchar.h>
#include <wchar.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <cctype>
#include <atomic>
#include <sstream>
#include <vector>
#include <stack>
#include <deque>
#include <unordered_map>
#include <algorithm>
#include <ctime>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iomanip>
#include <strstream>
#include <chrono>
#include <random>

#pragma comment(lib, "PSAPI.lib")
#pragma comment(lib, "COMCTL32.lib")
#pragma comment(lib, "UXTHEME.lib")
#pragma comment(lib, "Gdiplus.lib")
#pragma comment(lib, "Shlwapi.lib")

#pragma warning(disable : 4200)
#pragma warning(disable : 4996)
#pragma warning(disable : 4477)

#include "typedefs.hpp"
#include "debug.hpp"

#define assert ASSERT

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#define SetDlgItemString(hWnd, IdCtrl, IdString)                            \
    do                                                                      \
    {                                                                       \
        TCHAR tszBuffer[256];                                               \
        LoadString(NULL, IdString, tszBuffer, sizeof(tszBuffer));           \
        SetDlgItemText(hWnd, IdCtrl, tszBuffer);                            \
    } while(0)

#ifdef _UNICODE
namespace std
{
    typedef wstring tstring;
};
#else
namespace std
{
    typedef string tstring;
};
#endif

#define REF(p)          			(p)

#define STR(s)                      #s

#define COUNT_OF(ptr)	            int32(sizeof(ptr) / sizeof(ptr[0]))

#define BIT(no)			            (uint32(1 << (no)))

#define BITSOF(var)		            (sizeof(var) << 3)

#define BIT_SET(bitfield, no)       (bitfield |= (1 << no))

#define BIT_CLEAR(bitfield, no)     (bitfield &= ~(1 << no))

#define IS_BIT_SET(bitfield, no)    ((bitfield >> no) & 1)

#define FLAG_SET(maskfield, flag)\
	maskfield |= flag

#define FLAG_CLEAR(maskfield, flag)\
	maskfield &= ~flag

#define IS_FLAG_SET(flagfield, flag)\
	bool((flagfield & flag) == flag)

#define IS_FLAG_SET_ANY(flagfield, flag)\
	bool((flagfield & (flag)) != 0)

#define FLAG_CHANGE(flagfield, flag, set)	\
do											\
{											\
if (set)									\
	flagfield |= (flag);					\
else										\
	flagfield &= (~flag);					\
}											\
while (0)


template<class T>
inline T Clamp(T val, T min, T max)
{
    return (
        val > max ?
        max :
        (val < min ? min : val)
    );
};


template<class T>
inline T InvClamp(T val, T min, T max)
{
    return (
        val > max ?
        min :
        (val < min ? max : val)
    );
};


template<class T>
inline T Max(T a, T b)
{
    return (a > b ? a : b);
};


template<class T>
inline T Min(T a, T b)
{
    return (a < b ? a : b);
};