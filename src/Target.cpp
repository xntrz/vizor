#include "Target.hpp"


/*extern*/ Target_t Target =
{
    NULL,
    0,
    267696933
};


extern "C" bool PmInitialize(void);
extern "C" void PmTerminate(void);
extern "C" const char* PmRequestString(int32 No);


void TargetInitialize(void)
{
    PmInitialize();
};


void TargetTerminate(void)
{
    PmTerminate();
};


HWND TargetSearch(void)
{
    const char* pszString = PmRequestString(PString_User32);
    if (!pszString)
        return NULL;

    HMODULE hMod = GetModuleHandleA(pszString);

    pszString = PmRequestString(PString_FindWindow);
    if (!pszString)
        return NULL;
    
    typedef HWND(WINAPI *PFN_FINDWINDOWA)(LPCSTR lpszClassName, LPCSTR lpszWindowName);
    
    PFN_FINDWINDOWA pfnFindWindowA = PFN_FINDWINDOWA(GetProcAddress(hMod, pszString));
    if (!pfnFindWindowA)
        return NULL;

    pszString = PmRequestString(PString_WindowName);
    if (!pszString)        
        return NULL;

    return pfnFindWindowA(nullptr, pszString);
};


const wchar_t* TargetProtectedStringW(int32 No)
{
    thread_local static wchar_t wszBuffer[256];

    ASSERT(No >= 0 && No < PString_MAX);

    const char* pszString = PmRequestString(No);
    if (!pszString)
        return nullptr;

    std::mbstowcs(wszBuffer, pszString, std::strlen(pszString) + 1);

    return wszBuffer;
};


const char* TargetProtectedStringA(int32 No)
{
    return PmRequestString(No);
};


const TCHAR* TargetProtectedStringT(int32 No)
{
#ifdef _UNICODE
    return TargetProtectedStringW(No);
#else
    return TargetProtectedStringA(No);
#endif    
};