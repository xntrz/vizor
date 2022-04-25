#pragma once


struct Target_t
{
    HWND WindowHandle;
    uint32 WindowProcessID;
    uint32 ImageChecksum;
};


enum PString_t
{
    PString_ModuleName = 0,
    PString_WindowName,
    PString_User32,
    PString_FindWindow,
    PString_Opt1,
    PString_Opt2,
    
    PString_MAX,
};


extern Target_t Target;


void TargetInitialize(void);
void TargetTerminate(void);
HWND TargetSearch(void);
const wchar_t* TargetProtectedStringW(int32 No);
const char* TargetProtectedStringA(int32 No);
const TCHAR* TargetProtectedStringT(int32 No);