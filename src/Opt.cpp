#include "Opt.hpp"
#include "OptX.hpp"


void OptInitialize(void)
{
    VzOptInitialize();
};


void OptTerminate(void)
{
    VzOptTerminate();
};


void OptActivate(int32 OptNo)
{
    VzOptSendCmd(OptNo, VzOptCmd_Activate);
};


void OptDeactivate(int32 OptNo)
{
    VzOptSendCmd(OptNo, VzOptCmd_Deactivate);
};


bool OptIsEnable(int32 OptNo)
{
    return VzOptIsEnable(OptNo);
};


int32 OptGetNumAvailable(void)
{
    return VzOptGetNumAvailable();
};


OptBind_t OptGetBind(int32 OptNo)
{
    VzOptBind_t BindPrivate = VzOptGetBind(OptNo);    
    OptBind_t BindPublic = {};
    
    BindPublic.VkKey = BindPrivate.VkKey;
    BindPublic.VkMod = BindPrivate.VkMod;
    
    return BindPublic;
};


void OptSetBind(int32 OptNo, OptBind_t Bind)
{
    VzOptBind_t BindPrivate = {};

    BindPrivate.VkKey = Bind.VkKey;
    BindPrivate.VkMod = Bind.VkMod;

    VzOptSetBind(OptNo, BindPrivate);
};