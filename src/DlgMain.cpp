#include "DlgMain.hpp"
#include "VizorRc.hpp"
#include "File.hpp"
#include "Opt.hpp"
#include "Target.hpp"
#include "AsyncKey.hpp"
#include "Setting.hpp"


static const int32 WM_EOL = (WM_USER + 1);


struct OptInfo_t
{
    int32 IdPString;
    int32 IdCtrlChk;
    int32 IdCtrlKey;
    bool Status;
    bool Enable;
};


static OptInfo_t OptInfoTable[] =
{
	{ PString_Opt1, IDD_MAIN_OPT1_CHK, IDD_MAIN_OPT1_KEY, false, true },
    { PString_Opt2, IDD_MAIN_OPT2_CHK, IDD_MAIN_OPT2_KEY, false, true },
};


struct DlgMain_t
{
    HWND hWnd;
	uint32 BindPollThreadPPS;
    std::thread BindPollThread;
    bool BindPollThreadRunFlag;
    HANDLE TargetProcessHandle;
    bool EOLFlag;
};


static void ActivateOpt(int32 OptNo, bool bActivate)
{
    if (bActivate)
        OptActivate(OptNo);
    else
        OptDeactivate(OptNo);
};


static inline int32 SearchChkCtrlByKeyCtrl(int32 IdCtrlKey)
{
    for (int32 i = 0; i < COUNT_OF(OptInfoTable); ++i)
    {
        if (OptInfoTable[i].IdCtrlKey == IdCtrlKey)
            return OptInfoTable[i].IdCtrlChk;
    };

    return -1;
};


static inline int32 SearchKeyCtrlByChkCtrl(int32 IdCtrlChk)
{
    for (int32 i = 0; i < COUNT_OF(OptInfoTable); ++i)
    {
        if (OptInfoTable[i].IdCtrlChk == IdCtrlChk)
            return OptInfoTable[i].IdCtrlKey;
    };

    return -1;
};


static inline bool IsKeyCtrl(int32 IdCtrl)
{
    return (SearchChkCtrlByKeyCtrl(IdCtrl) != -1);
};


static inline bool IsChkCtrl(int32 IdCtrl)
{
    return (SearchKeyCtrlByChkCtrl(IdCtrl) != -1);
};


static inline int32 OptNoFromChkOrKey(int32 IdCtrl)
{
    for (int32 i = 0; i < COUNT_OF(OptInfoTable); ++i)
    {
        if (OptInfoTable[i].IdCtrlChk == IdCtrl ||
            OptInfoTable[i].IdCtrlKey == IdCtrl)
            return i;
    };

    return -1;
};


static inline int32 VkModToKeyMod(int32 VkMod)
{
    switch (VkMod)
    {
    case VK_MENU:
    case VK_RMENU:
    case VK_LMENU:
        return HOTKEYF_ALT;
        
    case VK_SHIFT:
    case VK_RSHIFT:
    case VK_LSHIFT:
        return HOTKEYF_SHIFT;

    case VK_CONTROL:
    case VK_RCONTROL:
    case VK_LCONTROL:
        return HOTKEYF_CONTROL;

    default:
        ASSERT(false);
        return -1;
    };
};


static inline int32 KeyModToVkMod(int32 KeyMod)
{
    switch (KeyMod)
    {
    case HOTKEYF_ALT:
        return VK_MENU;

    case HOTKEYF_CONTROL:
        return VK_CONTROL;

    case HOTKEYF_SHIFT:        
        return VK_SHIFT;

    case HOTKEYF_EXT:
        return 0;

    default:
        ASSERT(false);
        return -1;
    };
};


static void OptPollThread(DlgMain_t* Dlg)
{
	uint32 SleepTime = 1000 / Dlg->BindPollThreadPPS;

    while (Dlg->BindPollThreadRunFlag)
    {
        if (!Dlg->EOLFlag)
        {
            uint32 ExitCode = 0;
            if (GetExitCodeProcess(Dlg->TargetProcessHandle, LPDWORD(&ExitCode)))
            {
                if (ExitCode != STILL_ACTIVE)
                {
                    Dlg->EOLFlag = true;
                    PostMessage(Dlg->hWnd, WM_EOL, 0, 0);                    
                };
            };
        };

		int32 AvailableOptNum = OptGetNumAvailable();
		for (int32 i = 0; i < AvailableOptNum; ++i)
		{
			if (!OptIsEnable(i))
			{
				if (OptInfoTable[i].Enable)
				{
					HWND hWndChk = GetDlgItem(Dlg->hWnd, OptInfoTable[i].IdCtrlChk);
					HWND hWndKey = GetDlgItem(Dlg->hWnd, OptInfoTable[i].IdCtrlKey);
					
					EnableWindow(hWndChk, FALSE);
					EnableWindow(hWndKey, FALSE);

					OptInfoTable[i].Enable = false;
				};

				continue;
			}
			else
			{
				if (!OptInfoTable[i].Enable)
				{
					HWND hWndChk = GetDlgItem(Dlg->hWnd, OptInfoTable[i].IdCtrlChk);
					HWND hWndKey = GetDlgItem(Dlg->hWnd, OptInfoTable[i].IdCtrlKey);

					EnableWindow(hWndChk, TRUE);
					EnableWindow(hWndKey, TRUE);

					OptInfoTable[i].Enable = true;
				};
			};

			if ((GetForegroundWindow() == Target.WindowHandle) && (!Dlg->EOLFlag))
			{
				OptBind_t Bind = OptGetBind(i);
				bool bKeyStatus = false;

				if (Bind.VkKey && AsyncKeyIsReleased(Bind.VkKey))
					bKeyStatus = true;

				if (Bind.VkMod && !AsyncKeyIsHeld(Bind.VkMod))
					bKeyStatus = false;

				if (bKeyStatus)
				{
					OptInfoTable[i].Status = !OptInfoTable[i].Status;

					ActivateOpt(i, OptInfoTable[i].Status);

					HWND hWndChk = GetDlgItem(Dlg->hWnd, OptInfoTable[i].IdCtrlChk);
					SendMessage(
						hWndChk,
						BM_SETCHECK,
						(OptInfoTable[i].Status ? BST_CHECKED : BST_UNCHECKED),
						0
					);
				};
			};
		};

        std::this_thread::sleep_for(std::chrono::milliseconds(SleepTime));
    };
};


static bool DlgMainOnCreate(DlgMain_t* Dlg)
{
    Dlg->TargetProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, Target.WindowProcessID);
    Dlg->BindPollThreadRunFlag = true;
    Dlg->BindPollThreadPPS = 60;
    Dlg->BindPollThread = std::thread(OptPollThread, Dlg);

    for (int32 i = 0; i < COUNT_OF(OptInfoTable); ++i)
    {
        SetDlgItemText(
            Dlg->hWnd,
            OptInfoTable[i].IdCtrlChk,
            TargetProtectedStringT(OptInfoTable[i].IdPString)
        );
    };

    SetDlgItemString(Dlg->hWnd, IDD_MAIN_EXIT, IDS_EXIT);

    int32 AvailableOptNum = OptGetNumAvailable();
    ASSERT(AvailableOptNum == COUNT_OF(OptInfoTable), "update me");
    if (AvailableOptNum != COUNT_OF(OptInfoTable))
        return false;

    for (int32 i = 0; i < AvailableOptNum; ++i)
    {
        OptBind_t Bind = OptGetBind(i);
		if (Bind.VkMod)
			Bind.VkMod = VkModToKeyMod(Bind.VkMod);

        SendMessage(
            GetDlgItem(Dlg->hWnd, OptInfoTable[i].IdCtrlKey),
            HKM_SETHOTKEY,
            MAKEWORD(Bind.VkKey, Bind.VkMod),
            NULL
        );
    };
    
    return true;
};


static void DlgMainOnDestroy(DlgMain_t* Dlg)
{
    Dlg->BindPollThreadRunFlag = false;
    if (Dlg->BindPollThread.joinable())
        Dlg->BindPollThread.join();
    CloseHandle(Dlg->TargetProcessHandle);
};


static void DlgMainOnCommand(DlgMain_t* Dlg, int32 IdCtrl, HWND hWndCtrl, uint32 NotifyCode)
{
    if (IsKeyCtrl(IdCtrl))
    {
		uint32 Hotkey = uint32(SendMessage(hWndCtrl, HKM_GETHOTKEY, 0, 0));
        uint8 Vk = LOBYTE(LOWORD(Hotkey));
        uint8 Mod = HIBYTE(LOWORD(Hotkey));

		if(Mod)
			Mod = KeyModToVkMod(Mod);

		if (Vk)
		{
			int32 OptNo = OptNoFromChkOrKey(IdCtrl);
			ASSERT(OptNo != -1);

			OptBind_t Bind = {};
			Bind.VkKey = Vk;
			Bind.VkMod = Mod;

            OptSetBind(OptNo, Bind);
        };
    }
    else if (IsChkCtrl(IdCtrl))
    {
        int32 OptNo = OptNoFromChkOrKey(IdCtrl);
        ASSERT(OptNo != -1);
        ASSERT(OptNo >= 0 && OptNo < COUNT_OF(OptInfoTable));
        
        bool bResult = (SendMessage(hWndCtrl, BM_GETCHECK, 0, 0) > 0);
        
        OptInfoTable[OptNo].Status = !OptInfoTable[OptNo].Status;
        ActivateOpt(OptNo, bResult);
    }
    else
    {
        switch (IdCtrl)
        {
        case IDD_MAIN_EXIT:
            {
                EndDialog(Dlg->hWnd, 0);
            }
            break;
        };
    };
};


static BOOL CALLBACK DlgMainMsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    DlgMain_t* Dlg = (DlgMain_t*)GetWindowLongPtr(hWnd, DWLP_USER);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            SetWindowLong(hWnd, DWLP_USER, LONG(lParam));
            Dlg = (DlgMain_t*)GetWindowLongPtr(hWnd, DWLP_USER);
            Dlg->hWnd = hWnd;
            return DlgMainOnCreate(Dlg);
        };
        break;

    case WM_EOL:
        EndDialog(hWnd, 1);
		return true;
        
    case WM_CLOSE:
        DestroyWindow(hWnd);
        return true;

    case WM_DESTROY:
        DlgMainOnDestroy(Dlg);
        return true;        

    case WM_COMMAND:
        DlgMainOnCommand(Dlg, LOWORD(wParam), HWND(lParam), HIWORD(wParam));
        return true;

    default:
        return false;
    };
};


int32 DlgMainCreate(void)
{
    OptInitialize();

    DlgMain_t Dlg = {};

	int32 Result = int32(DialogBoxParam(
		NULL,
		MAKEINTRESOURCE(IDD_MAIN),
		GetDesktopWindow(),
		DLGPROC(DlgMainMsgProc),
		LPARAM(&Dlg)
    ));
    
    if (Result == -1)
        ASSERT(false, "%u", GetLastError());

    OptTerminate();

    return Result;
};