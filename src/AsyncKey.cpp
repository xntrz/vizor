#include "AsyncKey.hpp"


static bool PressStateTable[256];
static bool ReleaseStateTable[256];


bool AsyncKeyIsPressed(int32 VK)
{
    if (GetAsyncKeyState(VK))
    {
        if (!PressStateTable[VK])
        {
            PressStateTable[VK] = true;
            return true;
        };
    }
    else
    {
        PressStateTable[VK] = false;
    };

	return false;
};


bool AsyncKeyIsReleased(int32 VK)
{
    if (GetAsyncKeyState(VK))
    {
        if (!ReleaseStateTable[VK])
        {
            ReleaseStateTable[VK] = true;
            return false;
        };
    }
    else
    {
        if (ReleaseStateTable[VK])
        {
            ReleaseStateTable[VK] = false;
            return true;
        };
    };

    return false;
};


bool AsyncKeyIsHeld(int32 VK)
{
    SHORT State = GetAsyncKeyState(VK);
    return IS_FLAG_SET(State, 0x8000);
};