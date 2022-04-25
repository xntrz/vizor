#include "DlgHk.hpp"
#include "DlgMain.hpp"
#include "VizorRc.hpp"
#include "File.hpp"
#include "Target.hpp"


struct DlgHk_t
{
    HWND hWnd;
    int32 CheckPeriodNo;
    HWND CheckWindow;
    uint32 CheckProcessID;
};


static const int32 IDT_PERIOD = 100;


static void DlgHkTimerProc(DlgHk_t* Dlg, int32 IdTimer);


static bool DlgHkOnCreate(DlgHk_t* Dlg)
{
    SetDlgItemString(Dlg->hWnd, IDD_HK_EXIT, IDS_EXIT);
    SetDlgItemString(Dlg->hWnd, IDD_HK_LABEL, IDS_HK_PHASE1);

	DlgHkTimerProc(Dlg, IDT_PERIOD);
    SetTimer(Dlg->hWnd, IDT_PERIOD, 500, NULL);

    return true;
};


static void DlgHkOnDestroy(DlgHk_t* Dlg)
{
    KillTimer(Dlg->hWnd, IDT_PERIOD);
};


static void DlgHkOnCommand(DlgHk_t* Dlg, int32 IdCtrl)
{
	switch (IdCtrl)
	{
    case IDD_HK_EXIT:
        EndDialog(Dlg->hWnd, 0);
        break;
    };
};


static void DlgHkTimerProc(DlgHk_t* Dlg, int32 IdTimer)
{
    TCHAR tszBuffer[256];
	TCHAR tszText[256];
    
    GetDlgItemText(Dlg->hWnd, IDD_HK_LABEL, tszBuffer, sizeof(tszBuffer));

	int32 TextLen = _stprintf_s(
		tszText,
		sizeof(tszText) / sizeof(TCHAR),
		tszBuffer,
		TargetProtectedStringT(PString_ModuleName)
	);

	if (Dlg->CheckPeriodNo < 3)
	{
		tszText[TextLen + 1] = TEXT('\0');
		tszText[TextLen] = TEXT('.');
		++Dlg->CheckPeriodNo;
	}
	else
	{
		tszText[TextLen - 3] = TEXT('\0');
		Dlg->CheckPeriodNo = 0;
	};

	SetDlgItemText(Dlg->hWnd, IDD_HK_LABEL, tszText);

    HWND hWnd = TargetSearch();
    if (hWnd)
    {
        uint32 ProcessID = 0;
        uint32 ThreadID = GetWindowThreadProcessId(hWnd, PDWORD(&ProcessID));

        Dlg->CheckProcessID = ProcessID;
        Dlg->CheckWindow = hWnd;

        EndDialog(Dlg->hWnd, 1);
    };
};


static BOOL CALLBACK DlgHkMsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    DlgHk_t* Dlg = (DlgHk_t*)GetWindowLongPtr(hWnd, DWLP_USER);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            SetWindowLong(hWnd, DWLP_USER, LONG(lParam));
			Dlg = (DlgHk_t*)GetWindowLongPtr(hWnd, DWLP_USER);
            Dlg->hWnd = hWnd;
            return DlgHkOnCreate(Dlg);
        };
        break;

    case WM_CLOSE:
    case WM_DESTROY:
        DlgHkOnDestroy(Dlg);
        return true;
        
    case WM_COMMAND:
        DlgHkOnCommand(Dlg, LOWORD(wParam));
        return true;

    case WM_TIMER:
        DlgHkTimerProc(Dlg, int32(wParam));
        return true;

    default:
        return false;
    };
};


int32 DlgHkCreate(void)
{
    DlgHk_t Dlg = {};

    int32 Result = int32(DialogBoxParam(
        NULL,
        MAKEINTRESOURCE(IDD_HK),
        NULL,
        DLGPROC(DlgHkMsgProc),
        LPARAM(&Dlg)
    ));

    if (Result)
    {
        Target.WindowHandle = Dlg.CheckWindow;
        Target.WindowProcessID = Dlg.CheckProcessID;
    };

    return Result;
};