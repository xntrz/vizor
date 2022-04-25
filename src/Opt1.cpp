#include "OptX.hpp"
#include "MmKit.hpp"
#include "Target.hpp"


struct Opt1Container_t
{
    uint64 hHistory;
    bool Enable;
};


static Opt1Container_t Opt1Container;


void Opt1Proc(int32 Cmd, void* Param)
{
    switch (Cmd)
    {
    case VzOptCmd_Init:
        {
            Opt1Container.hHistory = MmKitPatchHistoryCreate();
            Opt1Container.Enable = true;
        }
        break;
        
    case VzOptCmd_Term:
        {
            MmKitPatchHistorySetCurrent(Opt1Container.hHistory);
            MmKitPatchHistoryUndo(Opt1Container.hHistory);
            MmKitPatchHistoryDestroy(Opt1Container.hHistory);
            Opt1Container.hHistory = 0;
        }
        break;
        
    case VzOptCmd_Activate:
        {
            const char Payload[] =
            {
                '\xB0', '\x01',     // mov al, 1
                '\x90',             // nop
                '\x90',             // nop
                '\x90'              // nop
            };

            const char* Module = TargetProtectedStringA(PString_ModuleName);
            ASSERT(Module);
            sizeof(HWND);
            MmKitPatchHistorySetCurrent(Opt1Container.hHistory);
            MmKitPatchAddress(Module, 0x718FC6, Payload, sizeof(Payload));
            MmKitPatchAddress(Module, 0x67AD71, Payload, sizeof(Payload));
            MmKitPatchAddress(Module, 0x62D0F3, Payload, sizeof(Payload));
            MmKitPatchAddress(Module, 0x5F4B86, Payload, sizeof(Payload));
            MmKitPatchAddress(Module, 0x1496FB8, Payload, sizeof(Payload));
            MmKitPatchAddress(Module, 0x1497051, Payload, sizeof(Payload));
        }
        break;

    case VzOptCmd_Deactivate:
        {
            MmKitPatchHistorySetCurrent(Opt1Container.hHistory);
            MmKitPatchHistoryUndo(Opt1Container.hHistory);
        }
        break;

    case VzOptCmd_CheckEnable:
        {
            *((bool*)Param) = Opt1Container.Enable;
        }
        break;
    };
};