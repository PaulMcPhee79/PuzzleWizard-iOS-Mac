
#include "DeviceDetails.h"
#include "cocos2d.h"
USING_NS_CC;

#ifdef __ANDROID__
	#include <CMJniUtils.h>
	static bool isInitialized = false;
	static float wInches = 0, hInches = 0;
#endif

unsigned int CM_getPlatformType(void)
{
#ifdef __ANDROID__
	return 0;
#else
	return UIDevice2GiPad;
#endif
}

bool isIPad(void)
{
#ifdef __ANDROID__
	if (!isInitialized)
	{
		wInches = CMJniUtils::getDisplayWidthInInches();
		hInches = CMJniUtils::getDisplayHeightInInches();
		isInitialized = true;
		CCLog("RESOLUTION: wInches:%f hInches:%f", wInches, hInches);
	}

	// Only use 10x8 if the screen is both physically large enough and has a reasonably square-shaped ratio.
	if (hInches == 0)
		return false;
	else
		return wInches > 4.4f && hInches > 3.4f && (wInches / hInches < 2.0f);
#else
    return true;
#endif
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
