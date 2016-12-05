
#include "cocos2d.h"
#include "DeviceDetails.h"
#include <Utils/UIDevice-Hardware.h>

#ifdef CHEEKY_MOBILE
static bool s_deviceTypeCached = false;
static uint s_deviceType;

unsigned int CM_getPlatformType(void)
{
    if (!s_deviceTypeCached)
    {
        s_deviceType = (uint)[UIDevice currentDevice].platformType;
        s_deviceTypeCached = true;
    }
    
    return s_deviceType;
}

static bool s_isIPadCached = false;
static bool s_isIPad = false;

bool isIPad(void)
{
    if (!s_isIPadCached)
    {
        s_isIPad = UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad;
        s_isIPadCached = true;
    }
    
    return s_isIPad;
}

bool is8x6(void)
{
    //return !isIPad();
    return false;
}

bool isTablet(void)
{
    return isIPad();
}
#else
unsigned int CM_getPlatformType(void)
{
    return UIDevice3GiPad;
}

bool isIPad(void)
{
    return true;
}

bool is8x6(void)
{
    return false;
}

bool isTablet(void)
{
    return isIPad();
}
#endif
