#include "DlgSel.hpp"
#include "VizorRc.hpp"
#include "DlgResize.hpp"
#include "Target.hpp"


struct DlgSel_t
{
    HWND hWnd;
    HWND CheckWindow;
    uint32 CheckProcessID;
    uint64 hDlgResize;
    WNDPROC OrgProcLV;
    HIMAGELIST hImageListLV;
    int32 ListSize;
	bool EnumWindowNameExeFlag;
	bool EnumWindowNameLenFlag;
	bool EnumWindowVisibleFlag;
};


static const TCHAR* LV_PROP_NAME = TEXT("__DlgSelInstanceProp");


static HICON EnumWindowGetIcon(HWND hWnd)
{
    uint32 Timeout = 1000;
    HICON hResult = NULL;

    SendMessageTimeout(hWnd, WM_GETICON, ICON_SMALL, 0, 0, Timeout, PDWORD_PTR(&hResult));

    if (hResult == NULL)
        SendMessageTimeout(hWnd, WM_GETICON, ICON_SMALL2, 0, 0, Timeout, PDWORD_PTR(&hResult));

    if (hResult == NULL)
        SendMessageTimeout(hWnd, WM_GETICON, ICON_BIG, 0, 0, Timeout, PDWORD_PTR(&hResult));

    if (hResult == NULL)
        hResult = HICON(GetClassLongPtr(hWnd, GCLP_HICON));

    if (hResult == NULL)
        hResult = HICON(GetClassLongPtr(hWnd, GCLP_HICONSM));

    if (hResult == NULL)
        hResult = LoadIcon(NULL, IDI_APPLICATION);

    if (hResult)
        hResult = CopyIcon(hResult);

    return hResult;
};


static BOOL WINAPI EnumWindowProc(HWND hWnd, LPARAM lParam)
{
    DlgSel_t* Dlg = (DlgSel_t*)lParam;
    TCHAR tszBuffer[256] = {};
    TCHAR* pszName = nullptr;
    HANDLE hProcess = NULL;
    uint32 ThreadID = 0;
    uint32 ProcessID = 0;
    bool bResult = false;

    if (Dlg->EnumWindowNameLenFlag)
    {
        int32 WindowNameLen = GetWindowTextLength(hWnd);
        if (WindowNameLen <= 0)
            return TRUE;
    };

    if (Dlg->EnumWindowVisibleFlag)
    {
        if (!IsWindowVisible(hWnd))
            return TRUE;
    };	

    ThreadID = GetWindowThreadProcessId(hWnd, LPDWORD(&ProcessID));
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessID);
    if (hProcess)
    {
        if (Dlg->EnumWindowNameExeFlag)
        {
            if (GetProcessImageFileName(hProcess, tszBuffer, sizeof(tszBuffer)))
            {
                pszName = _tcsrchr(tszBuffer, TEXT('\\'));
                ASSERT(pszName);
				++pszName;
                bResult = true;
            }
            else
            {
                bResult = false;
            };
        }
        else
        {
            GetWindowText(hWnd, tszBuffer, sizeof(tszBuffer));
            pszName = tszBuffer;
            bResult = true;
        };

        if (bResult)
        {
            HICON hIcon = EnumWindowGetIcon(hWnd);
            ASSERT(hIcon);

            TCHAR tszItemName[256] = {};
            _stprintf_s(
                tszItemName,
                sizeof(tszItemName) / sizeof(TCHAR),
                TEXT("0x%llx -- %s"),
                hWnd,
                pszName
            );

            int32 iImageIndex = ImageList_AddIcon(Dlg->hImageListLV, hIcon);

            LVITEM LvItem = {};
            LvItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
			LvItem.pszText = tszItemName;
            LvItem.iItem = Dlg->ListSize++;
            LvItem.iImage = iImageIndex;
            LvItem.lParam = LPARAM(hWnd);
            ListView_InsertItem(GetDlgItem(Dlg->hWnd, IDD_SEL_LV), &LvItem);

            DestroyIcon(hIcon);
        };

        CloseHandle(hProcess);
        hProcess = NULL;
    };

    return TRUE;
};


static void DlgSelEnumWindow(DlgSel_t* Dlg)
{
    Dlg->ListSize = 0;
    
    ImageList_RemoveAll(Dlg->hImageListLV);
    ListView_DeleteAllItems(GetDlgItem(Dlg->hWnd, IDD_SEL_LV));

    EnumWindows(EnumWindowProc, LPARAM(Dlg));
};


static void DlgSelSaveResult(DlgSel_t* Dlg)
{
    HWND hWndLV = GetDlgItem(Dlg->hWnd, IDD_SEL_LV);
    int32 CurrSel = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);
    
	LVITEM LvItem = {};
	LvItem.mask = LVIF_PARAM;
	LvItem.iItem = CurrSel;

    ListView_GetItem(hWndLV, &LvItem);

    Dlg->CheckWindow = HWND(LvItem.lParam);
    GetWindowThreadProcessId(Dlg->CheckWindow, LPDWORD(&Dlg->CheckProcessID));
};


static LRESULT DlgSelListViewProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DlgSel_t* DlgSel = (DlgSel_t*)GetProp(hWnd, LV_PROP_NAME);
	if (DlgSel)
	{
		switch (uMsg)
		{
		case WM_NOTIFY:
		{
			switch (LPNMHDR(lParam)->code)
			{
			case HDN_BEGINTRACK:
				return 1;
			};
		}
		break;
		};
	};

    return CallWindowProc(DlgSel->OrgProcLV, hWnd, uMsg, wParam, lParam);
};


static bool DlgSelOnCreate(DlgSel_t* Dlg)
{
    SetDlgItemString(Dlg->hWnd, IDD_SEL_OK, IDS_OK);
    SetDlgItemString(Dlg->hWnd, IDD_SEL_EXIT, IDS_EXIT);
    SetDlgItemString(Dlg->hWnd, IDD_SEL_UPDT, IDS_UPDATE);

    Dlg->hDlgResize = DlgResizeCreate(Dlg->hWnd);
    ASSERT(Dlg->hDlgResize);
    DlgResizeRegistEx(Dlg->hDlgResize, IDD_SEL_LV, DlgResizeFlag_Size);
    DlgResizeRegistEx(Dlg->hDlgResize, IDD_SEL_UPDT, DlgResizeFlag_Move);
    DlgResizeRegistEx(Dlg->hDlgResize, IDD_SEL_OK, DlgResizeFlag_Move);
    DlgResizeRegistEx(Dlg->hDlgResize, IDD_SEL_EXIT, DlgResizeFlag_Move);

    HWND hWndLV = GetDlgItem(Dlg->hWnd, IDD_SEL_LV);
    SetProp(hWndLV, LV_PROP_NAME, Dlg);
    Dlg->OrgProcLV = WNDPROC(SetWindowLongPtr(hWndLV, GWLP_WNDPROC, LONG_PTR(DlgSelListViewProc)));

    static TCHAR* apszColumnNames[] = { TEXT("Window") };
    LVCOLUMN LvColumn = {};
    LvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    for (int32 i = 0; i < sizeof(apszColumnNames) / sizeof(apszColumnNames[0]); ++i)
    {
        LvColumn.iSubItem = i;
        LvColumn.pszText = apszColumnNames[i];
        LvColumn.fmt = LVCFMT_CENTER;
        LvColumn.cx = -2;
        
        ListView_InsertColumn(hWndLV, i, &LvColumn);
    };

    SendMessage(hWndLV, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
    SendMessage(hWndLV, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_GRIDLINES, LVS_EX_GRIDLINES);
    SendMessage(hWndLV, LVM_SETCOLUMNWIDTH, 0, LVSCW_AUTOSIZE_USEHEADER);
    SetWindowTheme(hWndLV, L"Explorer", NULL);
	
    Dlg->hImageListLV = ImageList_Create(16, 16, ILC_COLOR32, 0, 128);
    ASSERT(Dlg->hImageListLV);
    ListView_SetImageList(hWndLV, Dlg->hImageListLV, LVSIL_SMALL);

    return true;
};


static void DlgSelOnDestroy(DlgSel_t* Dlg)
{
    DlgSelSaveResult(Dlg);
    
    if (Dlg->hImageListLV)
    {
        ImageList_Destroy(Dlg->hImageListLV);
        Dlg->hImageListLV = NULL;
    };
    
    HWND hWndLV = GetDlgItem(Dlg->hWnd, IDD_SEL_LV);
    SetWindowLongPtr(hWndLV, GWLP_WNDPROC, LONG_PTR(Dlg->OrgProcLV));
    RemoveProp(hWndLV, LV_PROP_NAME);

    if (Dlg->hDlgResize)
    {
        DlgResizeDestroy(Dlg->hDlgResize);
        Dlg->hDlgResize = 0;
    };
};


static void DlgSelOnNotify(DlgSel_t* Dlg, int32 IdCtrl, NMHDR* pInfo)
{
    switch (IdCtrl)
    {
    case IDD_SEL_LV:
        {
            switch (pInfo->code)
            {
            case NM_DBLCLK:
                {
                    PostMessage(Dlg->hWnd, WM_COMMAND, IDD_SEL_OK, 0);
                }
                break;
            };
        }
        break;
    };
};


static void DlgSelOnCommand(DlgSel_t* Dlg, int32 IdCtrl)
{
    switch (IdCtrl)
    {
    case IDD_SEL_OK:
        {
            EndDialog(Dlg->hWnd, 1);
        }
        break;

    case IDD_SEL_EXIT:
        {
            EndDialog(Dlg->hWnd, 0);
        }
        break;
        
    case IDD_SEL_UPDT:
        {
            DlgSelEnumWindow(Dlg);
        }
        break;
    };
};


static BOOL CALLBACK DlgSelMsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    DlgSel_t* Dlg = (DlgSel_t*)GetWindowLongPtr(hWnd, DWLP_USER);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            SetWindowLong(hWnd, DWLP_USER, LONG(lParam));
            Dlg = (DlgSel_t*)GetWindowLongPtr(hWnd, DWLP_USER);
            Dlg->hWnd = hWnd;
            return DlgSelOnCreate(Dlg);
        };
		return false;

    case WM_SIZE:
        {
            HWND hWndLV = GetDlgItem(Dlg->hWnd, IDD_SEL_LV);
            SendMessage(hWndLV, LVM_SETCOLUMNWIDTH, 0, LVSCW_AUTOSIZE_USEHEADER);
        }
        return false;

    case WM_CLOSE:
        DestroyWindow(hWnd);
        return true;
        
    case WM_DESTROY:
        DlgSelOnDestroy(Dlg);
        return true;

    case WM_NOTIFY:
        DlgSelOnNotify(Dlg, int32(wParam), LPNMHDR(lParam));
		return true;

    case WM_COMMAND:
        DlgSelOnCommand(Dlg, LOWORD(wParam));
        return true;

    default:
        return false;
    };
};


int32 DlgSelCreate(void)
{
    DlgSel_t Dlg = {};
    Dlg.EnumWindowNameExeFlag = true;
    Dlg.EnumWindowNameLenFlag = true;
    Dlg.EnumWindowVisibleFlag = true;
    
    int32 Result = int32(DialogBoxParam(
        NULL,
        MAKEINTRESOURCE(IDD_SEL),
        NULL,
        DLGPROC(DlgSelMsgProc),
        LPARAM(&Dlg)
    ));

    if (Result)
    {
        Target.WindowHandle = Dlg.CheckWindow;
        Target.WindowProcessID = Dlg.CheckProcessID;
    };

    return Result;
};