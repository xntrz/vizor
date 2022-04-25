#include "OptX.hpp"
#include "MmKit.hpp"
#include "Target.hpp"


struct Opt2Container_t
{
    uint64 hHistory;
    bool Enable;
};


static Opt2Container_t Opt2Container;


void Opt2Proc(int32 Cmd, void* Param)
{
    switch (Cmd)
    {
    case VzOptCmd_Init:
        {
            Opt2Container.hHistory = MmKitPatchHistoryCreate();
            Opt2Container.Enable = true;
        }
        break;

    case VzOptCmd_Term:
        {
            MmKitPatchHistorySetCurrent(Opt2Container.hHistory);
            MmKitPatchHistoryUndo(Opt2Container.hHistory);
            MmKitPatchHistoryDestroy(Opt2Container.hHistory);
            Opt2Container.hHistory = 0;
        }
        break;

    case VzOptCmd_Activate:
        {
            const char Payload[] =
            {
                '\xC3', // retn
            };

            const char* Module = TargetProtectedStringA(PString_ModuleName);
            ASSERT(Module);

            MmKitPatchHistorySetCurrent(Opt2Container.hHistory);
            MmKitPatchAddress(Module, 0x4D9C20, Payload, sizeof(Payload));
        }
        break;

    case VzOptCmd_Deactivate:
        {
            MmKitPatchHistorySetCurrent(Opt2Container.hHistory);
            MmKitPatchHistoryUndo(Opt2Container.hHistory);
        }
        break;

    case VzOptCmd_CheckEnable:
        {
            *((bool*)Param) = Opt2Container.Enable;
        }
        break;
    };
};