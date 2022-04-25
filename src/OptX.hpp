#pragma once


enum VzOptCmd_t
{
    //
    //  Param - nullptr
    //
    VzOptCmd_Init = 0,

    //
    //  Param - nullptr
    //
    VzOptCmd_Term,

    //
    //  Param - nullptr
    //
    VzOptCmd_Activate,

    //
    //  Param - nullptr
    //
    VzOptCmd_Deactivate,

    //
    //  Param - указатель на bool флаг
    //
    VzOptCmd_CheckEnable,
};


struct VzOptBind_t
{
    uint16 VkKey;
    uint16 VkMod;
};


struct VzOptAsyncResult_t
{
    void* Context;    
    void* UserParam;
};


void VzOptInitialize(void);
void VzOptTerminate(void);
void VzOptSendCmd(int32 OptNo, int32 Cmd, void* Param = nullptr);
void VzOptSetBind(int32 OptNo, VzOptBind_t bind);
VzOptBind_t VzOptGetBind(int32 OptNo);
bool VzOptIsEnable(int32 OptNo);
int32 VzOptGetNumAvailable(void);