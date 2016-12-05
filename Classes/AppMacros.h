#ifndef __APPMACROS_H__
#define __APPMACROS_H__

#include "cocos2d.h"
#include <Utils/DeviceDetails.h>

/* For demonstrating using one design resolution to match different resources,
   or one resource to match different design resolutions.

   [Situation 1] Using one design resolution to match different resources.
     Please look into Appdelegate::applicationDidFinishLaunching.
     We check current device frame size to decide which resource need to be selected.
     So if you want to test this situation which said in title '[Situation 1]',
     you should change ios simulator to different device(e.g. iphone, iphone-retina3.5, iphone-retina4.0, ipad, ipad-retina),
     or change the window size in "proj.XXX/main.cpp" by "CCEGLView::setFrameSize" if you are using win32 or linux plaform
     and modify "proj.mac/AppController.mm" by changing the window rectangle.

   [Situation 2] Using one resource to match different design resolutions.
     The coordinates in your codes is based on your current design resolution rather than resource size.
     Therefore, your design resolution could be very large and your resource size could be small.
     To test this, just define the marco 'TARGET_DESIGN_RESOLUTION_SIZE' to 'DESIGN_RESOLUTION_2048X1536'
     and open iphone simulator or create a window of 480x320 size.

   [Note] Normally, developer just need to define one design resolution(e.g. 960x640) with one or more resources.
 */

//#define DESIGN_RESOLUTION_480X320    0
//#define DESIGN_RESOLUTION_1024X768   1
//#define DESIGN_RESOLUTION_2048X1536  2
//#define DESIGN_RESOLUTION_CUSTOM     3

/* If you want to switch design resolution, change next line */
#define TARGET_DESIGN_RESOLUTION_SIZE DESIGN_RESOLUTION_CUSTOM //  DESIGN_RESOLUTION_CUSTOM //DESIGN_RESOLUTION_480X320

typedef struct tagResource
{
    cocos2d::CCSize size;
    char directory[100];
}Resource;

#ifdef CHEEKY_MOBILE

	#ifdef __ANDROID__
		static Resource smallResource_8x6 = { cocos2d::CCSizeMake(480, 320), "display/res_low" };
		static Resource smallResource_10x8 = { cocos2d::CCSizeMake(512, 384), "display/res_low" };
		static Resource* smallResource = NULL;

		static Resource mediumResource_8x6 = { cocos2d::CCSizeMake(960, 640), "display/res_med" };
		static Resource mediumResource_10x8 = { cocos2d::CCSizeMake(1024, 768), "display/res_med" };
		static Resource* mediumResource = NULL;

		static Resource largeResource_8x6 = { cocos2d::CCSizeMake(1920, 1280), "display/res_hi" };
		static Resource largeResource_10x8 = { cocos2d::CCSizeMake(2048, 1536), "display/res_hi" };
		static Resource* largeResource = NULL;
	#else
		static Resource smallResource_8x6 = { cocos2d::CCSizeMake(480, 320), "iphone" };
		static Resource smallResource_10x8 = { cocos2d::CCSizeMake(512, 384), "iphone" };
		static Resource* smallResource = NULL;

		static Resource mediumResource_8x6 = { cocos2d::CCSizeMake(960, 640), "ipad+iphonehd" };
		static Resource mediumResource_10x8 = { cocos2d::CCSizeMake(1024, 768), "ipad+iphonehd" };
		static Resource* mediumResource = NULL;

		static Resource largeResource_8x6 = { cocos2d::CCSizeMake(1920, 1280), "ipadhd" };
		static Resource largeResource_10x8 = { cocos2d::CCSizeMake(2048, 1536), "ipadhd" };
		static Resource* largeResource = NULL;
	#endif

#else
		static Resource osxResource  =  { cocos2d::CCSizeMake(2048, 1536), "machd" };
#endif

#ifdef CHEEKY_MOBILE
		static cocos2d::CCSize designResolutionSize_8x6 = cocos2d::CCSizeMake(960, 640);
		static cocos2d::CCSize designResolutionSize_10x8 = cocos2d::CCSizeMake(1024, 768);
#else
		static cocos2d::CCSize designResolutionSize_10x8 = cocos2d::CCSizeMake(1024, 768);
#endif
static cocos2d::CCSize* designResolutionSize = NULL;

static void initAppMacros(void)
{
#ifdef CHEEKY_MOBILE
	smallResource = MODE_8x6 ? &smallResource_8x6 : &smallResource_10x8;
	mediumResource = MODE_8x6 ? &mediumResource_8x6 : &mediumResource_10x8;
	largeResource = MODE_8x6 ? &largeResource_8x6 : &largeResource_10x8;
	designResolutionSize = MODE_8x6 ? &designResolutionSize_8x6 : &designResolutionSize_10x8;
#else
    designResolutionSize = &designResolutionSize_10x8;
#endif
}

//#if (TARGET_DESIGN_RESOLUTION_SIZE == DESIGN_RESOLUTION_480X320)
//static cocos2d::CCSize designResolutionSize = cocos2d::CCSizeMake(480, 320);
//#elif (TARGET_DESIGN_RESOLUTION_SIZE == DESIGN_RESOLUTION_1024X768)
//static cocos2d::CCSize designResolutionSize = cocos2d::CCSizeMake(1024, 768);
//#elif (TARGET_DESIGN_RESOLUTION_SIZE == DESIGN_RESOLUTION_2048X1536)
//static cocos2d::CCSize designResolutionSize = cocos2d::CCSizeMake(2048, 1536);
//#elif (TARGET_DESIGN_RESOLUTION_SIZE == DESIGN_RESOLUTION_CUSTOM)
//	#if MODE_8x6
//		static cocos2d::CCSize designResolutionSize = cocos2d::CCSizeMake(960, 640);
//	#else
//		static cocos2d::CCSize designResolutionSize = cocos2d::CCSizeMake(1024, 768);
//	#endif
//#else
//#error unknown target design resolution!
//#endif

// The font size 24 is designed for small resolution, so we should change it to fit for current design resolution
#define TITLE_FONT_SIZE  (cocos2d::CCEGLView::sharedOpenGLView()->getDesignResolutionSize().width / smallResource.size.width * 24)

#endif /* __APPMACROS_H__ */
