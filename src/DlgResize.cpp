#include "DlgResize.hpp"
#include "List.hpp"


struct DlgResizeNode_t
{
    ListNode_t Node;
    HWND hWnd;
    DlgResizeFlag_t Flags;
};


struct DlgResize_t
{
    HWND Target;
    DLGPROC OrgProc;
    List_t NodeList;
    int32 PrevW;
    int32 PrevH;
    int32 MinW;
    int32 MinH;
};


static const TCHAR* RESIZE_PROP_NAME = TEXT("__DlgResizeInstanceProp");


static void HandleResize(DlgResizeNode_t* ResizeNode, HWND hWndTarget, int32 DeltaX, int32 DeltaY)
{
    UINT SwpFlags = SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOMOVE;
    RECT RcScreen = { 0 };
    RECT RcClient = { 0 };
    uint32 Flags = ResizeNode->Flags;
    
    GetWindowRect(ResizeNode->hWnd, &RcScreen);
    GetClientRect(ResizeNode->hWnd, &RcClient);
    MapWindowPoints(ResizeNode->hWnd, hWndTarget, LPPOINT(&RcClient), sizeof(RcClient) / sizeof(POINT));

    if (IS_FLAG_SET_ANY(Flags, DlgResizeFlag_X | DlgResizeFlag_Y))
        FLAG_CLEAR(SwpFlags, SWP_NOMOVE);

    if (IS_FLAG_SET(Flags, DlgResizeFlag_X))
        RcClient.left += DeltaX;

    if (IS_FLAG_SET(Flags, DlgResizeFlag_Y))
        RcClient.top += DeltaY;

    if (IS_FLAG_SET(Flags, DlgResizeFlag_CX))
        RcScreen.right += DeltaX;

    if (IS_FLAG_SET(Flags, DlgResizeFlag_CY))
        RcScreen.bottom += DeltaY;

    SetWindowPos(
        ResizeNode->hWnd,
        NULL,
        RcClient.left,
        RcClient.top,
        RcScreen.right  - RcScreen.left,
        RcScreen.bottom - RcScreen.top,
        SwpFlags
    );
};


static LRESULT WINAPI DlgResizeProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    DlgResize_t* DlgResize = (DlgResize_t*)GetProp(hWnd, RESIZE_PROP_NAME);
    ASSERT(DlgResize);
    
    switch (uMsg)
    {
    case WM_GETMINMAXINFO:
        {
            LPMINMAXINFO pMinMaxInfo = LPMINMAXINFO(lParam);
            pMinMaxInfo->ptMinTrackSize.x = DlgResize->MinW;
            pMinMaxInfo->ptMinTrackSize.y = DlgResize->MinH;
        }
        break;

    case WM_SIZE:
        {
            int32 W = LOWORD(lParam);
            int32 H = HIWORD(lParam);
            int32 DeltaX = W - DlgResize->PrevW;
            int32 DeltaY = H - DlgResize->PrevH;

            DlgResizeNode_t* ResizeNode = (DlgResizeNode_t*)DlgResize->NodeList.Head;
            while (ResizeNode)
            {
                HandleResize(ResizeNode, DlgResize->Target, DeltaX, DeltaY);
                ResizeNode = (DlgResizeNode_t*)ResizeNode->Node.Next;
            };

            DlgResize->PrevW = W;
            DlgResize->PrevH = H;

            InvalidateRect(hWnd, NULL, FALSE);
        }
        break;
    };
    
    return CallWindowProc(DlgResize->OrgProc, hWnd, uMsg, wParam, lParam);
};


uint64 DlgResizeCreate(HWND hWndTarget)
{
    DlgResize_t* DlgResize = new DlgResize_t();
    if (!DlgResize)
        return 0;

    RECT RcMinSize = {};
    GetWindowRect(hWndTarget, &RcMinSize);
    DlgResize->MinW = (RcMinSize.right - RcMinSize.left);
    DlgResize->MinH = (RcMinSize.bottom - RcMinSize.top);

    RECT RcPrevSize = {};
    GetClientRect(hWndTarget, &RcPrevSize);
    DlgResize->PrevW = (RcPrevSize.right - RcPrevSize.left);
    DlgResize->PrevH = (RcPrevSize.bottom - RcPrevSize.top);

    SetProp(hWndTarget, RESIZE_PROP_NAME, DlgResize);
    DlgResize->OrgProc = DLGPROC(SetWindowLongPtr(hWndTarget, DWLP_DLGPROC, LONG_PTR(DlgResizeProc)));
    DlgResize->Target = hWndTarget;

    ListInit(&DlgResize->NodeList);

    return uint64(DlgResize);
};


void DlgResizeDestroy(uint64 hResize)
{
    DlgResize_t* DlgResize = (DlgResize_t*)hResize;

	DlgResizeNode_t* ResizeNode = (DlgResizeNode_t*)DlgResize->NodeList.Head;
    while (ResizeNode)
    {
        DlgResizeNode_t* Next = (DlgResizeNode_t*)ResizeNode->Node.Next;

        ListRemove(&DlgResize->NodeList, (ListNode_t*)ResizeNode);
        delete ResizeNode;

        ResizeNode = Next;
    };

    SetWindowLongPtr(DlgResize->Target, DWLP_DLGPROC, LONG_PTR(DlgResize->OrgProc));
    RemoveProp(DlgResize->Target, RESIZE_PROP_NAME);

    delete DlgResize;
};


void DlgResizeRegist(uint64 hResize, HWND hWnd, DlgResizeFlag_t Flags)
{
    DlgResize_t* DlgResize = (DlgResize_t*)hResize;
    
    DlgResizeNode_t* ResizeNode = new DlgResizeNode_t();
    if (ResizeNode)
    {
        ResizeNode->hWnd = hWnd;
        ResizeNode->Flags = Flags;

        ListInsert(&DlgResize->NodeList, (ListNode_t*)ResizeNode);
    };
};


void DlgResizeRegistEx(uint64 hResize, int32 IdCtrl, DlgResizeFlag_t Flags)
{
    DlgResize_t* DlgResize = (DlgResize_t*)hResize;
  
    DlgResizeRegist(hResize, GetDlgItem(DlgResize->Target, IdCtrl), Flags);
};


void DlgResizeRemove(uint64 hResize, HWND hWnd)
{
    DlgResize_t* DlgResize = (DlgResize_t*)hResize;

    DlgResizeNode_t* ResizeNode = (DlgResizeNode_t*)DlgResize->NodeList.Head;
    while (ResizeNode)
    {
        if (ResizeNode->hWnd == hWnd)
        {
            ListRemove(&DlgResize->NodeList, (ListNode_t*)ResizeNode);
            delete ResizeNode;
            break;
        };
          
        ResizeNode = (DlgResizeNode_t*)ResizeNode->Node.Next;
    };
};