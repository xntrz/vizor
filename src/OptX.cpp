#include "OptX.hpp"
#include "File.hpp"
#include "MmKit.hpp"
#include "List.hpp"
#include "version.hpp"
#include "Setting.hpp"


static const TCHAR* OPTSAVEFILE = TEXT("VizorSave.dat");
static const int32 OPTNUM = 12;
static const int32 OPTSEC = OPTNUM;


typedef void(*OptProc)(int32 OptCmd, void* Param);


struct VzOptObject_t
{
    OptProc OptProcTable[OPTNUM + 1];
    VzOptBind_t OptBindVk[OPTNUM];
    bool OptEnableStatus[OPTNUM];
    int32 OptNum;
    bool NoSecFlag;
};


extern void Opt1Proc(int32 Cmd, void* Param);
extern void Opt2Proc(int32 Cmd, void* Param);
extern void OptSecProc(int32 Cmd, void* Param);


static VzOptObject_t VzOptObject =
{
    {
        Opt1Proc,
        Opt2Proc,
    },

    {
        { VK_NUMPAD1 },
        { VK_NUMPAD2 },
    },
};


void VzOptInitialize(void)
{
    MmKitInitialize();

    for (int32 i = 0; i < OPTNUM; ++i)
    {
        if (VzOptObject.OptProcTable[i])
        {
            VzOptObject.OptEnableStatus[i] = true;

            ++VzOptObject.OptNum;
            VzOptSendCmd(i, VzOptCmd_Init);
        };
    };

    void* hFile = VzFOpen(OPTSAVEFILE, TEXT("rb"));
    if (hFile)
    {
        int16 VersionMajor = 0;
        int16 VersionMinor = 0;
        
        VzFRead(hFile, (int8*)&VersionMajor, sizeof(VersionMajor));
        VzFRead(hFile, (int8*)&VersionMinor, sizeof(VersionMinor));

        if (VersionMajor == APP_VER_MAJOR &&
            VersionMinor == APP_VER_MINOR)
        {
            int32 BindNum = 0;
            VzFRead(hFile, (int8*)&BindNum, sizeof(BindNum));

            if (BindNum == VzOptObject.OptNum)
                VzFRead(hFile, (int8*)&VzOptObject.OptBindVk[0], sizeof(VzOptObject.OptBindVk));
        };
        
        VzFClose(hFile);
    };

    VzOptObject.NoSecFlag = SettingIsPresent("nosec");
    if(!VzOptObject.NoSecFlag)
    {
        VzOptObject.OptProcTable[OPTSEC] = OptSecProc;
        VzOptSendCmd(OPTSEC, VzOptCmd_Init);
        VzOptSendCmd(OPTSEC, VzOptCmd_Activate);
    };
};


void VzOptTerminate(void)
{
    if (!VzOptObject.NoSecFlag)
    {
        VzOptSendCmd(OPTSEC, VzOptCmd_Deactivate);
        VzOptSendCmd(OPTSEC, VzOptCmd_Term);
    };
    
    void* hFile = VzFOpen(OPTSAVEFILE, TEXT("wb"));
    if (hFile)
    {
        int16 VersionMajor = APP_VER_MAJOR;
        int16 VersionMinor = APP_VER_MINOR;        
        VzFWrite(hFile, (int8*)&VersionMajor, sizeof(VersionMajor));
        VzFWrite(hFile, (int8*)&VersionMinor, sizeof(VersionMinor));
        VzFWrite(hFile, (int8*)&VzOptObject.OptNum, sizeof(VzOptObject.OptNum));
        VzFWrite(hFile, (int8*)&VzOptObject.OptBindVk[0], sizeof(VzOptObject.OptBindVk));
        VzFClose(hFile);
    };

    while (VzOptObject.OptNum)
    {
        VzOptSendCmd(VzOptObject.OptNum - 1, VzOptCmd_Term);
        --VzOptObject.OptNum;
    };

    MmKitTerminate();
};


void VzOptSendCmd(int32 OptNo, int32 Cmd, void* Param)
{
    ASSERT((OptNo >= 0 && OptNo < VzOptObject.OptNum) || (OptNo == OPTSEC));
    ASSERT(VzOptObject.OptProcTable[OptNo]);
    VzOptObject.OptProcTable[OptNo](Cmd, Param);
};


void VzOptSetBind(int32 OptNo, VzOptBind_t bind)
{
    ASSERT(OptNo >= 0 && OptNo < VzOptObject.OptNum);
    VzOptObject.OptBindVk[OptNo] = bind;
};


VzOptBind_t VzOptGetBind(int32 OptNo)
{
    ASSERT(OptNo >= 0 && OptNo < VzOptObject.OptNum);
    return VzOptObject.OptBindVk[OptNo];
};


bool VzOptIsEnable(int32 OptNo)
{
    ASSERT(OptNo >= 0 && OptNo < VzOptObject.OptNum);
    bool bCheckFlag = false;
    VzOptSendCmd(OptNo, VzOptCmd_CheckEnable, &bCheckFlag);
    return bCheckFlag;
};


int32 VzOptGetNumAvailable(void)
{
    return VzOptObject.OptNum;
};