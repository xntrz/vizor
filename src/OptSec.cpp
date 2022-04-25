#include "OptX.hpp"
#include "MmKit.hpp"
#include "Target.hpp"


struct OptSecContainer_t
{
    uint64 hHistory;
    bool Enable;
};


static OptSecContainer_t OptSecContainer;


void OptSecProc(int32 Cmd, void* Param)
{
    switch (Cmd)
    {
    case VzOptCmd_Init:
        {
            OptSecContainer.hHistory = MmKitPatchHistoryCreate();
            OptSecContainer.Enable = true;
        }
        break;

    case VzOptCmd_Term:
        {
            MmKitPatchHistorySetCurrent(OptSecContainer.hHistory);
            MmKitPatchHistoryUndo(OptSecContainer.hHistory);
            MmKitPatchHistoryDestroy(OptSecContainer.hHistory);
            OptSecContainer.hHistory = 0;
        }
        break;

    case VzOptCmd_Activate:
        {
            const char Payload0[] =
            {
                '\x90', '\x90', '\x90', '\x90', '\x90', '\x90'
            };

            const char Payload1[] =
            {
                '\x33', '\xC0', // xor rax, rax
                '\xC3',         // retn
            };

            const char* Module = TargetProtectedStringA(PString_ModuleName);
            ASSERT(Module);

            MmKitPatchHistorySetCurrent(OptSecContainer.hHistory);
            MmKitPatchAddress(Module, 0x1EAF307, Payload0, sizeof(Payload0));
            MmKitPatchAddress(Module, 0xCC1C10, Payload1, sizeof(Payload1));
        }
        break;

    case VzOptCmd_Deactivate:
        {
            MmKitPatchHistorySetCurrent(OptSecContainer.hHistory);
            MmKitPatchHistoryUndo(OptSecContainer.hHistory);
        }
        break;

    case VzOptCmd_CheckEnable:
        {
            *((bool*)Param) = OptSecContainer.Enable;
        }
        break;
    };
};