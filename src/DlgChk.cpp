#include "DlgChk.hpp"
#include "Target.hpp"
#include "File.hpp"
#include "VizorRc.hpp"


int32 DlgChkCreate(void)
{
    int32 Result = 0;

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, Target.WindowProcessID);
    if (hProcess)
    {
        TCHAR tszImagePath[MAX_PATH];
        tszImagePath[0] = TEXT('\0');
        GetModuleFileNameEx(hProcess, NULL, tszImagePath, sizeof(tszImagePath));

        void* hFile = VzFOpen(tszImagePath, TEXT("rb"));
        if (hFile)
        {
            uint32 ChkSum = VzFChkSumm(hFile);            
            VzFClose(hFile);

            if (ChkSum == Target.ImageChecksum)
            {
                Result = 1;
            }
            else
            {
                TCHAR tszText[256];
                TCHAR tszTitle[256];

                LoadString(NULL, IDS_WARNING, tszTitle, sizeof(tszTitle));
                LoadString(NULL, IDS_HK_ERR_CHKSUM, tszText, sizeof(tszText));

                int32 MsgResult = MessageBox(NULL, tszText, tszTitle, MB_ICONWARNING | MB_OKCANCEL);
                if (MsgResult == IDOK)
                    Result = 1;
            };
        };

        CloseHandle(hProcess);
    };

    return Result;
};