#include "DlgSeq.hpp"
#include "DlgSel.hpp"
#include "DlgHk.hpp"
#include "DlgMain.hpp"
#include "DlgChk.hpp"
#include "Setting.hpp"


enum DlgSequence_t
{
    DlgSequence_Hook = 0,
    DlgSequence_Select,
    DlgSequence_Check,
    DlgSequence_Menu,
    DlgSequence_EOL,
};


static int32 DlgSequence = DlgSequence_Hook;


static void DlgSeqStrideNext(int32 Seq, int32 DlgRet)
{
    int32 NextSeq = DlgSequence_EOL;

    switch (DlgSequence)
    {
    case DlgSequence_Hook:
    case DlgSequence_Select:
        {
            if (DlgRet)
                NextSeq = DlgSequence_Check;
        }
        break;

    case DlgSequence_Check:
        {
            if (DlgRet)
                NextSeq = DlgSequence_Menu;
            else
                NextSeq = DlgSequence_Hook;
        }
        break;

    case DlgSequence_Menu:
        {
            if (DlgRet)
                NextSeq = DlgSequence_Hook;                        
        }
        break;
    };

    DlgSequence = NextSeq;
};


void DlgSeqRun(void)
{
    int32 DlgRet = 0;

    while (DlgSequence != DlgSequence_EOL)
    {
        switch (DlgSequence)
        {
        case DlgSequence_Hook:
        case DlgSequence_Select:
            {
                if (SettingIsPresent("select"))
                    DlgRet = DlgSelCreate();
                else
                    DlgRet = DlgHkCreate();

                DlgSeqStrideNext(DlgSequence_Check, DlgRet);
            }
            break;

        case DlgSequence_Check:
            {
                DlgRet = DlgChkCreate();
                DlgSeqStrideNext(DlgSequence_Menu, DlgRet);
            }
            break;

        case DlgSequence_Menu:
            {
                DlgRet = DlgMainCreate();
                DlgSeqStrideNext(DlgSequence_EOL, DlgRet);
            }
            break;
        };
    };
};
