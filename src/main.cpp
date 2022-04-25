#include "DlgSeq.hpp"
#include "Setting.hpp"
#include "Target.hpp"

#ifdef _VLD_CHECK
#include "vld.h"
#endif


int32
APIENTRY
_tWinMain(
    HINSTANCE	hInstance,
    HINSTANCE	hPrevInstance,
    LPTSTR		lpCmdLine,
    int32		iCmdShow
)
{
    TCHAR szModulePath[MAX_PATH] = { 0 };
    TCHAR* pszResult = nullptr;

    GetModuleFileName(NULL, szModulePath, sizeof(szModulePath));
    pszResult = _tcsrchr(szModulePath, TEXT('\\'));
    *pszResult = TEXT('\0');
    SetCurrentDirectory(szModulePath);
    
    if (SettingIsCheckSumShowCall())
        return 0;

#ifdef _DEBUG
    DbgInit();
#endif

#ifdef _VLD_CHECK
    VLDEnable();
#endif

	TargetInitialize();
    DlgSeqRun();
	TargetTerminate();

#ifdef _DEBUG
    DbgTerm();
#endif
    
    return 0;
};
