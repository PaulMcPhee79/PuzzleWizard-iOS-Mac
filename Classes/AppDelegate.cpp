#include "AppDelegate.h"

#include <vector>
#include <string>

#ifdef __APPLE__
#include "../../../../CocosDenshion/include/SimpleAudioEngine.h"
#elif defined (__ANDROID__)
#include "SimpleAudioEngine.h"
#else
#include "../CocosDenshion/include/SimpleAudioEngine.h"
#endif

#include <SceneControllers/GameController.h>
#include <Utils/DeviceDetails.h>
#include <Utils/BridgingUtility.h>
#include <Managers/GameSettings.h>
#include <Puzzle/View/Playfield/Effects/TileRotator.h> // For shadow occlusion hack on low-res devices
#include "AppMacros.h"
#include <Utils/UtilsNS.h>
USING_NS_CC;
using namespace std;

//#ifdef __ANDROID__
//#include <iostream>
//#include <iomanip>
//
//#include <unwind.h>
//#include <dlfcn.h>
//
//#include <sstream>
//#include <android/log.h>
//
//#include <signal.h>
//
//struct BacktraceState
//{
//    void** current;
//    void** end;
//};
//
//static _Unwind_Reason_Code unwindCallback(struct _Unwind_Context* context, void* arg)
//{
//    BacktraceState* state = static_cast<BacktraceState*>(arg);
//    uintptr_t pc = _Unwind_GetIP(context);
//    if (pc) {
//        if (state->current == state->end) {
//            return _URC_END_OF_STACK;
//        } else {
//            *state->current++ = reinterpret_cast<void*>(pc);
//        }
//    }
//    return _URC_NO_REASON;
//}
//
//size_t captureBacktrace(void** buffer, size_t max)
//{
//    BacktraceState state = {buffer, buffer + max};
//    _Unwind_Backtrace(unwindCallback, &state);
//
//    return state.current - buffer;
//}
//
//void dumpBacktrace(std::ostream& os, void** addrs, size_t count)
//{
//    for (size_t idx = 0; idx < count; ++idx) {
//        const void* addr = addrs[idx];
//        const char* symbol = "";
//
//        Dl_info info;
//        if (dladdr(addr, &info) && info.dli_sname) {
//            symbol = info.dli_sname;
//        }
//
//        os << "  #" << std::setw(2) << idx << ": " << addr << "  " << symbol << "\n";
//    }
//}
//
//void backtraceToLogcat()
//{
//    const size_t max = 30;
//    void* buffer[max];
//    std::ostringstream oss;
//
//    dumpBacktrace(oss, buffer, captureBacktrace(buffer, max));
//
//    __android_log_print(ANDROID_LOG_INFO, "app_name", "%s", oss.str().c_str());
//}
//
//static struct sigaction old_sa[NSIG];
//void signalHandler(int signal, siginfo_t* info, void* reserved)
//{
//	backtraceToLogcat();
//	abort();
//	//printf("---CHEEKYMAMMOTH SIGNAL HANDLER---\n");
//
//	//old_sa[signal].sa_handler(signal);
//}
//#endif


AppDelegate::AppDelegate()
{

}

AppDelegate::~AppDelegate() 
{
	CocosDenshion::SimpleAudioEngine::sharedEngine()->end();
}

//static void onUncaughtException(NSException *exception)
//{
//    // Just performs whatever saving/restoration steps we usually do when the play session may finish early
//	GameController::GC()->applicationDidEnterBackground();
//}

bool AppDelegate::applicationDidFinishLaunching() {
//#if defined(__APPLE__) && defined(CHEEKY_MOBILE)
//    NSSetUncaughtExceptionHandler(&onUncaughtException);
//#endif
    
#ifdef CHEEKY_MOBILE
    BridgingUtility::startSession(true);
    BridgingUtility::enableStatusBarLightColorStyle(true);
#endif

//#ifdef __ANDROID__
//    struct sigaction handler;
//    memset(&handler, 0, sizeof(handler));
//    handler.sa_sigaction = signalHandler;
//    handler.sa_flags = SA_RESETHAND;
//#define CATCHSIG(X) sigaction(X, &handler, &old_sa[X])
//    CATCHSIG(SIGILL);
//    CATCHSIG(SIGABRT);
//    CATCHSIG(SIGBUS);
//    CATCHSIG(SIGFPE);
//    CATCHSIG(SIGSEGV);
//    CATCHSIG(SIGINT);
//    CATCHSIG(SIGSTKFLT);
//    CATCHSIG(SIGPIPE);
//#endif

    // initialize cocos2d
    initAppMacros();
    BridgingUtility::configureAudioMode();
    CCDirector* pDirector = CCDirector::sharedDirector();
    CCEGLView* pEGLView = CCEGLView::sharedOpenGLView();

    pDirector->setOpenGLView(pEGLView);

    // Set the design resolution
#if defined(__APPLE__) || defined(__ANDROID__)
    pEGLView->setDesignResolutionSize(pDirector->getWinSizeInPixels().width, pDirector->getWinSizeInPixels().height, kResolutionNoBorder);
#else
	pEGLView->setDesignResolutionSize(designResolutionSize->width, designResolutionSize->height, kResolutionNoBorder);
#endif
	CCSize frameSize = pEGLView->getFrameSize();
   
    vector<string> searchPath;

#ifdef CHEEKY_MOBILE

        // In this demo, we select resource according to the frame's height.
        // If the resource size is different from design resolution size, you need to set contentScaleFactor.
        // We use the ratio of resource's height to the height of design resolution,
        // this can make sure that the resource's height could fit for the height of design resolution.

        // if the frame's height is larger than the height of medium resource size, select large resource.
        if (frameSize.height > (1.25f * mediumResource->size.height))
        {
            searchPath.push_back(largeResource->directory);
            pDirector->setContentScaleFactor(MIN(largeResource->size.height/designResolutionSize->height, largeResource->size.width/designResolutionSize->width));
            TileRotator::setShadowOccOverlap(1.0f);
            CCLog("Hi Def");
        }
        // if the frame's height is larger than the height of small resource size, select medium resource.
        else if (frameSize.height > (1.25f * smallResource->size.height))
        {
            searchPath.push_back(mediumResource->directory);
            pDirector->setContentScaleFactor(MIN(mediumResource->size.height/designResolutionSize->height, mediumResource->size.width/designResolutionSize->width));
            TileRotator::setShadowOccOverlap(1.0f);
            CCLog("Med Def");
        }
        // if the frame's height is smaller than the height of medium resource size, select small resource. (Edit: Removed small resources. Now shrinking medium resources).
        else
        {
            searchPath.push_back(mediumResource->directory);
            pDirector->setContentScaleFactor(MIN(mediumResource->size.height/designResolutionSize->height, mediumResource->size.width/designResolutionSize->width));
            TileRotator::setShadowOccOverlap(0.6f);
            CCLog("Low Def");
        }

#else
    searchPath.push_back(osxResource.directory);
    pDirector->setContentScaleFactor(MIN(osxResource.size.height/designResolutionSize->height, osxResource.size.width/designResolutionSize->width));
    TileRotator::setShadowOccOverlap(1.0f);
#endif

    searchPath.push_back("audio");
    searchPath.push_back("data");
    searchPath.push_back("data/locales");
    searchPath.push_back("particles");

//#ifdef __APPLE__
//	//pDirector->setContentScaleFactor(1.0f);
//    pDirector->setContentScaleFactor(MIN(customResource.size.height/designResolutionSize->height, customResource.size.width/designResolutionSize->width));
//#else
//    pDirector->setContentScaleFactor(MIN(customResource.size.height/designResolutionSize->height, customResource.size.width/designResolutionSize->width));
//#endif

    // set searching path
    CCFileUtils::sharedFileUtils()->setSearchPaths(searchPath);
	
    // turn on display FPS
    pDirector->setDisplayStats(false); // true on Android will flood the Garbage Collector
    pDirector->setDepthTest(false);

    double fps = 60;
    
#ifdef CHEEKY_MOBILE
    #if CM_LOW_POWER
        fps = 30;
    #endif
#endif

//#if defined(__APPLE__) && defined(CHEEKY_MOBILE)
//    unsigned int deviceType = CM_getPlatformType();
//    switch (deviceType)
//    {
//        case UIDevice1GiPhone:
//        case UIDevice3GiPhone:
//        case UIDevice3GSiPhone:
//        case UIDevice4iPhone:
//        case UIDevice1GiPod:
//        case UIDevice2GiPod:
//        case UIDevice3GiPod:
//        case UIDevice4GiPod:
//        case UIDevice1GiPad:
//            fps = 30;
//            break;
//        case UIDevice4SiPhone:
//        case UIDevice5iPhone:
//        case UIDevice2GiPad:
//        case UIDevice3GiPad:
//        case UIDevice4GiPad:
//        case UIDeviceUnknowniPhone:
//        case UIDeviceUnknowniPod:
//        case UIDeviceUnknowniPad:
//        default:
//            fps = 60;
//            break;
//    }
//#endif

    pDirector->setAnimationInterval(1.0 / fps);

    if (CCUserDefault::sharedUserDefault()->getBoolForKey(CMSettings::B_FULLSCREEN, true))
        BridgingUtility::setFullscreen(true);

	GameController::GC()->setFps(fps);

    return true;
}

// This function will be called when the app is inactive. When comes a phone call,it's be invoked too
void AppDelegate::applicationDidEnterBackground() {
#ifdef CHEEKY_MOBILE
    BridgingUtility::pauseEngine();
    GameController::GC()->applicationDidEnterBackground();
#endif
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground() {
#ifdef CHEEKY_MOBILE
    BridgingUtility::resumeEngine();
    GameController::GC()->applicationWillEnterForeground();
#endif
}
