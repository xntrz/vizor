#pragma once


struct OptBind_t
{
    uint16 VkKey;
    uint16 VkMod;
};


void OptInitialize(void);
void OptTerminate(void);
void OptActivate(int32 OptNo);
void OptDeactivate(int32 OptNo);
bool OptIsEnable(int32 OptNo);
int32 OptGetNumAvailable(void);
OptBind_t OptGetBind(int32 OptNo);
void OptSetBind(int32 OptNo, OptBind_t Bind);