#pragma once


enum DlgResizeFlag_t
{
    DlgResizeFlag_X     = BIT(0),
    DlgResizeFlag_Y     = BIT(1),
    DlgResizeFlag_CX    = BIT(2),
    DlgResizeFlag_CY    = BIT(3),
    DlgResizeFlag_Move  = DlgResizeFlag_X  | DlgResizeFlag_Y,
    DlgResizeFlag_Size  = DlgResizeFlag_CX | DlgResizeFlag_CY,
};


uint64 DlgResizeCreate(HWND hWndTarget);
void DlgResizeDestroy(uint64 hResize);
void DlgResizeRegist(uint64 hResize, HWND hWnd, DlgResizeFlag_t Flags);
void DlgResizeRegistEx(uint64 hResize, int32 IdCtrl, DlgResizeFlag_t Flags);
void DlgResizeRemove(uint64 hResize, HWND hWnd);