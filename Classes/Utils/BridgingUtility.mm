
#ifdef CHEEKY_MOBILE
    #if CM_EVLOG
        #define FLURRY_ENABLED 0
        #define TESTFLIGHT_ENABLED 0
    #else
        #define FLURRY_ENABLED 0
        #define TESTFLIGHT_ENABLED 0
    #endif

    #define CRASHLYTICS_ENABLED 0
#else
    #define CRASHLYTICS_ENABLED 0
    #define FLURRY_ENABLED 0
    #define TESTFLIGHT_ENABLED 0
#endif

#import <Foundation/Foundation.h>
#include "BridgingUtility.h"
#include <Utils/UIDevice-Hardware.h>
#import "DeviceDetails.h"
#include <Utils/CMTypes.h>
#include <Utils/Utils.h>
#include <SceneControllers/SceneController.h>
#include <Interfaces/IEventListener.h>
#import "BridgingHelper.h"
#import "cocos2d.h"
#import "ProxyUtility.h"
#import "EAGLView.h"
#import "CMiTunesManager.h"
#import "LevelMenu.h"
#ifdef CHEEKY_MOBILE
    #import "AlertView.h"
    #define ALERT_VIEW AlertView
#else
    #import "AlertViewNS.h"
    #define ALERT_VIEW AlertViewNS
#endif
#include <Managers/GameSettings.h>
#include <Utils/Localizer.h>

#if CRASHLYTICS_ENABLED
    #import <Crashlytics/Crashlytics.h>
#endif

#if FLURRY_ENABLED
    #import "Flurry.h"
#endif

#if TESTFLIGHT_ENABLED
    #import "TestFlight.h"
#endif

#if CM_BETA
    #import "TFFeedbackViewController.h"
    #import "TFWelcomeViewController.h"
    #import "TFThanksViewController.h"
#endif
USING_NS_CC;

static bool didBackgroundAudioComplete = false;
static BridgingHelper* bridgingHelper = nil;
static const size_t kEngineLockLenMax = 64;
static char g_engineLock[kEngineLockLenMax+1] = "";
//static NSString* g_engineLock = nil;

static NSString* cstr2String(const char* cstr)
{
    if (cstr)
        return [NSString stringWithCString:cstr encoding:NSUTF8StringEncoding];
    else
        return @"";
}

BridgingHelper* getBridgingHelper(void)
{
    if (bridgingHelper == nil)
        bridgingHelper = [[BridgingHelper alloc] init];
    return bridgingHelper;
}

void BridgingUtility::onBackgroundAudioCompleted(void)
{
    didBackgroundAudioComplete = true;
}

void BridgingUtility::configureAudioMode(void)
{
    [getBridgingHelper() configureAudioMode];
}

void BridgingUtility::setAudioMode(void)
{
    [getBridgingHelper() setAudioMode];
}

bool BridgingUtility::attemptAudioRestart(void)
{
    return [getBridgingHelper() attemptAudioRestart];
}

void BridgingUtility::applyDefaultResignActiveBehavior(void)
{
    [getBridgingHelper() applyDefaultResignActiveBehavior];
}

void BridgingUtility::enableBackgroundAudioCompletionListening(bool enable)
{
    if (enable)
        [getBridgingHelper() registerAudioCompletionCallback:BridgingUtility::onBackgroundAudioCompleted];
    else
        [getBridgingHelper() registerAudioCompletionCallback:NULL];
}

bool BridgingUtility::didBackGroundAudioComplete(bool reset)
{
    bool retval = didBackgroundAudioComplete;
    if (reset) didBackgroundAudioComplete = false;
    return retval;
}

bool BridgingUtility::isMainThread(void)
{
    return [[NSThread currentThread] isMainThread];
}

bool BridgingUtility::isFullscreen(void)
{
#ifdef CHEEKY_DESKTOP
    EAGLView* pView = [EAGLView sharedEGLView];
	return pView.isFullScreen;
#else
    return true;
#endif
}

void BridgingUtility::setFullscreen(bool value)
{
#ifdef CHEEKY_DESKTOP
    if (value != BridgingUtility::isFullscreen())
    {
        EAGLView* pView = [EAGLView sharedEGLView];
        [pView setFullScreen:value];
        //[AppController resolutionDidChange];
        ProxyUtility::resolutionDidChange();
    }
#endif
}

bool BridgingUtility::isIOSFeatureSupported(const char* reqSysVer)
{
#if defined(__APPLE__) && defined(CHEEKY_MOBILE)
    if (reqSysVer == nil)
        return false;
    NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
    return ([currSysVer compare:[NSString stringWithUTF8String:reqSysVer] options:NSNumericSearch] != NSOrderedAscending);
#else
    return false;
#endif
}

bool BridgingUtility::shouldPromptForRating(void)
{
    return [CMiTunesManager shouldPromptForRating];
}

void BridgingUtility::userRespondedToRatingPrompt(bool response)
{
    [CMiTunesManager userRespondedToRatingPrompt:response];
}

void BridgingUtility::userDidRateTheGame(bool success)
{
    [CMiTunesManager userDidRateTheGame:success];
}

std::vector<std::string> BridgingUtility::getPreferredLanguageCodes(void)
{
    std::vector<std::string> prefLangVec;
    NSArray* preferredLanguages = [NSLocale preferredLanguages];
    
    if (preferredLanguages)
    {
        for (id obj in preferredLanguages)
        {
            if ([obj isKindOfClass:[NSString class]])
            {
                NSString* prefLangCode = (NSString*)obj;
                if (prefLangCode && prefLangCode.length >= 2)
                {
                    prefLangCode = [prefLangCode substringToIndex:2];
                    prefLangVec.push_back([prefLangCode UTF8String]);
                }
            }
        }
    }
    
    return prefLangVec;
}

std::string BridgingUtility::getSplashImagePath(void)
{
    std::string splashPath;
#if defined(__APPLE__) && defined(CHEEKY_MOBILE)
    if ([UIScreen mainScreen])
    {
        std::map<int, std::string> splashMap;
        splashMap[480] = "Default";
        splashMap[960] = "Default@2x";
        splashMap[1136] = "Default-568h@2x";
        splashMap[1024] = "Default-Portrait~ipad";
        splashMap[2048] = "Default-Portrait@2x~ipad";
        
        int key = (int)([UIScreen mainScreen].scale * ([[UIScreen mainScreen] bounds].size.height + 0.01f));
        std::map<int, std::string>::iterator it = splashMap.find(key);
        if (it != splashMap.end())
            splashPath = it->second;
    }
    
    if (splashPath.empty())
    {
        unsigned int platformType = CM_getPlatformType();
        splashPath = platformType == UIDevice1GiPad || platformType == UIDevice2GiPad
        ? "Default-Portrait~ipad"
        : (platformType == UIDevice3GiPad || platformType == UIDevice4GiPad
           ? "Default-Portrait@2x~ipad"
           : (platformType == UIDevice5iPhone || platformType == UIDevice5SiPhone || platformType == UIDevice5GiPod
              ? "Default-568h@2x"
              : (platformType == UIDevice4GiPod || platformType == UIDevice4iPhone
                 ? "Default@2x"
                 : "Default")));
    }
#endif
    return splashPath;
}

bool BridgingUtility::isHighPerformanceMachine(void)
{
#ifdef __APPLE__
    #ifdef CHEEKY_MOBILE
    // Specific to Puzzle Wizard's shader demands.
    unsigned int platformType = CM_getPlatformType();
    return (
            platformType != UIDevice1GiPhone &&
            platformType != UIDevice3GiPhone &&
            platformType != UIDevice4iPhone &&
            platformType != UIDevice1GiPod &&
            platformType != UIDevice2GiPod &&
            platformType != UIDevice1GiPad);
    #else
        return true;
    #endif
#else
    return false;
#endif
}

bool BridgingUtility::isPad(void)
{
#ifdef __APPLE__
    #ifdef CHEEKY_MOBILE
        return UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad;
    #else
        return false;
    #endif
#else
    return false;
#endif
}

void BridgingUtility::postNotification(const char* name)
{
    if (name)
        [[NSNotificationCenter defaultCenter] postNotificationName:[NSString stringWithCString:name encoding:NSUTF8StringEncoding] object:nil];
}

void BridgingUtility::openURL(const char* url)
{
    if (url)
    {
        NSString* nsStr = cstr2String(url);
        if ([nsStr length] != 0)
        {
            NSURL* nsurl = [NSURL URLWithString:nsStr];
            if (nsurl)
            {
#ifdef CHEEKY_MOBILE
                [[UIApplication sharedApplication] openURL:nsurl];
#else
                [[NSWorkspace sharedWorkspace] openURL:nsurl];
#endif
            }
        }
    }
}

void BridgingUtility::openFileURL(const char* url)
{
#ifdef CHEEKY_DESKTOP
    if (url)
    {
        NSString* nsStr = cstr2String(url);
        if ([nsStr length] != 0)
        {
            NSURL* nsurl = [NSURL fileURLWithPath:nsStr];
            if (nsurl)
                [[NSWorkspace sharedWorkspace] openURL:nsurl];
        }
    }
#endif
}

void BridgingUtility::setIdleTimerDisabled(bool disabled)
{
#ifdef CHEEKY_MOBILE
    [[UIApplication sharedApplication] setIdleTimerDisabled:(disabled ? YES : NO)];
#endif
}

void BridgingUtility::setStatusBarHidden(bool hidden)
{
#ifdef CHEEKY_MOBILE
    [[UIApplication sharedApplication] setStatusBarHidden:hidden ? YES : NO];
#endif
}

void BridgingUtility::enableStatusBarLightColorStyle(bool enable)
{
#ifdef CHEEKY_MOBILE
    [[UIApplication sharedApplication] setStatusBarStyle:UIStatusBarStyleLightContent];
#endif
}

std::string BridgingUtility::localizedString(const char* str)
{
    if (str)
    {
        NSString* nsStr = NSLocalizedString([NSString stringWithCString:str encoding:NSUTF8StringEncoding], nil);
        if (nsStr && [nsStr length] > 0)
            return [nsStr UTF8String];
    }

    return "";
}

int BridgingUtility::alertButtonReturnToZeroBasedIndex(int value)
{
#ifdef CHEEKY_DESKTOP
    return value - (int)NSAlertFirstButtonReturn;
#else
    return value;
#endif
}

void BridgingUtility::showAlertView(IEventListener* listener, int tag, const std::vector<std::string>& config, bool cancelButton)
{
    if (listener)
    {
        ALERT_VIEW* view = [[[ALERT_VIEW alloc] initWithListener:listener cancelButton:cancelButton] autorelease];
        view.tag = tag;
        
        // Set title and message (if any)
        if (config.size() > 0)
        {
            if (!config[0].empty())
                [view setTitle:cstr2String(config[0].c_str())];
        }
        
        if (config.size() > 1)
        {
            if (!config[1].empty())
                [view setMessage:cstr2String(config[1].c_str())];
        }
        
        // Add buttons
        if (config.size() > 2)
        {
            for (std::vector<std::string>::const_iterator it = config.begin()+2; it != config.end(); ++it)
            {
                if (!it->empty())
                    [view addButtonWithTitle:cstr2String(it->c_str())];
            }
        }
        
        [view show];
    }
}

// ************************************************************************************
// Common public interface
const char* BridgingUtility::CRASH_CONTEXT_GAME_STATE = "GAME_STATE";
const char* BridgingUtility::CRASH_CONTEXT_LEVEL_NAME = "LEVEL_NAME";
const char* BridgingUtility::CRASH_CONTEXT_PUZZLE_NAME = "PUZZLE_NAME";
const char* BridgingUtility::CRASH_CONTEXT_LOCALE = "LOCALE";
const char* BridgingUtility::CRASH_CONTEXT_MENU = "MENU";
const char* BridgingUtility::CRASH_CONTEXT_SFX = "SFX";
const char* BridgingUtility::CRASH_CONTEXT_MUSIC = "MUSIC";
const char* BridgingUtility::CRASH_CONTEXT_ICLOUD = "ICLOUD";
const char* BridgingUtility::CRASH_CONTEXT_COLOR_BLIND = "COLOR_BLIND";
const char* BridgingUtility::CONTEXT_ENABLED = "Enabled";
const char* BridgingUtility::CONTEXT_DISABLED = "Disabled";

const char* BridgingUtility::EV_NAME_LOCALE_CHANGED = "LOCALE_CHANGED";
const char* BridgingUtility::EV_NAME_LEVEL_UNLOCKED = "LEVEL_UNLOCKED";
const char* BridgingUtility::EV_NAME_LEVEL_COMPLETED = "LEVEL_COMPLETED";
const char* BridgingUtility::EV_NAME_PUZZLE_ATTEMPTED = "PUZZLE_ATTEMPTED";
const char* BridgingUtility::EV_NAME_PUZZLE_SOLVED = "PUZZLE_SOLVED";
const char* BridgingUtility::EV_NAME_RATED_THE_GAME = "RATED_THE_GAME";
const char* BridgingUtility::EV_NAME_BETA_PUZZLE_BUGGED = "PUZZLE_BUGGED";
const char* BridgingUtility::EV_NAME_BETA_SUGGEST_IQ_ = "SUGGEST_IQ_";

const char* BridgingUtility::bool2Context(bool value)
{
    return value ? CONTEXT_ENABLED : CONTEXT_DISABLED;
}

void BridgingUtility::startSession(bool reportCrashes)
{
    FL_startSession(reportCrashes);
    TF_takeOff();
    CL_start();
}

void BridgingUtility::initCustomKeys(void)
{
    CL_initCustomKeys();
}

void BridgingUtility::addCustomEnvironmentInformation(const char* information, const char* key)
{
    TF_addCustomEnvironmentInformation(information, key);
}

void BridgingUtility::setUserIdentifier(const char* userid)
{
    FL_setUserID(userid);
    CL_setUserIdentifier(userid);
}

void BridgingUtility::setUserName(const char* username)
{
    CL_setUserName(username);
}

void BridgingUtility::setUserEmail(const char* email)
{
    CL_setUserEmail(email);
}

void BridgingUtility::logEvent(const char* ev)
{
    FL_logEvent(ev);
    CL_log(ev);
    TF_passCheckpoint(ev);
}

void BridgingUtility::logEvent(const char* ev, const std::map<std::string, std::string>& params)
{
    FL_logEvent(ev, params);
}

void BridgingUtility::logTimedEvent(const char* ev)
{
    FL_logTimedEvent(ev);
}

void BridgingUtility::logTimedEvent(const char* ev, const std::map<std::string, std::string>& params)
{
    FL_logTimedEvent(ev, params);
}

void BridgingUtility::endTimedEvent(const char* ev)
{
    FL_endTimedEvent(ev);
    TF_passCheckpoint(ev);
}

void BridgingUtility::endTimedEvent(const char* ev, const std::map<std::string, std::string>& params)
{
    FL_endTimedEvent(ev, params);
    TF_passCheckpoint(ev);
}

void BridgingUtility::setCrashContext(const char* value, const char* key)
{
    CL_setStringValue(value, key);
}

void BridgingUtility::submitFeedback(const char* feedback)
{
    TF_submitFeedback(feedback);
}

void BridgingUtility::forceCrash(void)
{
    CL_crash();
}

void BridgingUtility::pauseEngine(void)
{
#ifdef CHEEKY_MOBILE
    CCDirector::sharedDirector()->stopAnimation();
    CCDirector::sharedDirector()->pause();
#endif
}

bool BridgingUtility::pauseEngine(const char* lock)
{
    if (lock && strlen(lock) <= kEngineLockLenMax && *g_engineLock == '\0')
    {
        strcpy(g_engineLock, lock);
        BridgingUtility::pauseEngine();
        return true;
    } else
        return false;

//    if (lock && g_engineLock == nil)
//    {
//        g_engineLock = [[NSString alloc] initWithCString:lock encoding:NSUTF8StringEncoding];
//        BridgingUtility::pauseEngine();
//        return true;
//    } else
//        return false;
}

void BridgingUtility::resumeEngine(void)
{
    if (*g_engineLock == '\0')
    {
        CCDirector::sharedDirector()->resume();
        CCDirector::sharedDirector()->startAnimation();
    }
    
//    if (g_engineLock == nil)
//    {
//        CCDirector::sharedDirector()->resume();
//        CCDirector::sharedDirector()->startAnimation();
//    }
}

bool BridgingUtility::resumeEngine(const char* unlock)
{
    if (*g_engineLock && unlock && strcmp(g_engineLock, unlock) == 0)
    {
        *g_engineLock = '\0';
        BridgingUtility::resumeEngine();
        return true;
    } else
        return false;
    
//    if (g_engineLock && unlock && [g_engineLock isEqualToString:[NSString stringWithCString:unlock encoding:NSUTF8StringEncoding]])
//    {
//        g_engineLock = nil;
//        BridgingUtility::resumeEngine();
//        return true;
//    } else
//        return false;
}


// Beta UI
#if CM_BETA
static UIViewController* topViewControllerWithRootViewController(UIViewController* rootViewController)
{
    if ([rootViewController isKindOfClass:[UITabBarController class]]) {
        UITabBarController* tabBarController = (UITabBarController*)rootViewController;
        return topViewControllerWithRootViewController(tabBarController.selectedViewController);
    } else if ([rootViewController isKindOfClass:[UINavigationController class]]) {
        UINavigationController* navigationController = (UINavigationController*)rootViewController;
        return topViewControllerWithRootViewController(navigationController.visibleViewController);
    } else if (rootViewController.presentedViewController) {
        UIViewController* presentedViewController = rootViewController.presentedViewController;
        return topViewControllerWithRootViewController(presentedViewController);
    } else {
        return rootViewController;
    }
}

static UIViewController* getTopMostController(void)
{
    return topViewControllerWithRootViewController([UIApplication sharedApplication].keyWindow.rootViewController);
}

static UIViewController* getTopMostModalController(void)
{
    UIViewController* topController = [UIApplication sharedApplication].keyWindow.rootViewController;
    while (topController.presentedViewController)
        topController = topController.presentedViewController;
    return topController;
}

void BridgingUtility::showWelcomeView(void)
{
//    UIViewController* mainController = getTopMostModalController();
//    if (mainController)
//    {
//        CCDirector::sharedDirector()->stopAnimation();
//        CCDirector::sharedDirector()->pause();
//        
//        TFWelcomeViewController* welcomeController = [[[TFWelcomeViewController alloc] init] autorelease];
//        [mainController presentViewController:welcomeController animated:YES completion:nil];
//    }
}

void BridgingUtility::showThanksView(void)
{
//    UIViewController* mainController = getTopMostModalController();
//    if (mainController)
//    {
//        CCDirector::sharedDirector()->stopAnimation();
//        CCDirector::sharedDirector()->pause();
//        
//        TFThanksViewController* thanksController = [[[TFThanksViewController alloc] init] autorelease];
//        [mainController presentViewController:thanksController animated:YES completion:nil];
//    }
}
#else
void BridgingUtility::showWelcomeView(void) { }
void BridgingUtility::showThanksView(void) { }
#endif
// ************************************************************************************

// Flurry
#if FLURRY_ENABLED
static NSDictionary* evParams2Dictionary(const std::map<std::string, std::string>& params)
{
    if (params.empty())
        return nil;
    NSMutableDictionary* dict = [NSMutableDictionary dictionaryWithCapacity:(NSUInteger)params.size()];
    for (EvLogParamKV::const_iterator it = params.begin(); it != params.end(); ++it)
        [dict setObject:cstr2String(it->second.c_str())
                 forKey:cstr2String(it->first.c_str())];
    return [NSDictionary dictionaryWithDictionary:dict];
         
}

void BridgingUtility::FL_startSession(bool reportCrashes)
{
#if !CRASHLYTICS_ENABLED
    if (reportCrashes)
        [Flurry setCrashReportingEnabled:YES];
#endif
    
#if CM_BETA
    [Flurry startSession:@"2B59JXRKY6TDVV7RFSH5"];
#else
    [Flurry startSession:@"65QPFJ36WR5MDSPGHSDT"];
#endif
}

void BridgingUtility::FL_setUserID(const char* userid)
{
    if (userid)
    {
        NSString* useridStr = cstr2String(userid);
        NSLog(@"FL_setUserID:%@", useridStr);
        [Flurry setUserID:useridStr];
    }
}

void BridgingUtility::FL_logEvent(const char* ev)
{
    if (ev)
        [Flurry logEvent:cstr2String(ev)];
}

void BridgingUtility::FL_logEvent(const char* ev, const std::map<std::string, std::string>& params)
{
    if (ev)
    {
        NSString* evName = cstr2String(ev);
        NSDictionary* dictParams = evParams2Dictionary(params);
        if (dictParams)
            NSLog(@"FL_logEvent:%@ Params:%@", evName, dictParams);
        
        if (dictParams == nil)
            FL_logEvent(ev);
        else
            [Flurry logEvent:evName withParameters:dictParams];
    }
}

void BridgingUtility::FL_logTimedEvent(const char* ev)
{
    if (ev)
        [Flurry logEvent:cstr2String(ev) timed:YES];
}

void BridgingUtility::FL_logTimedEvent(const char* ev, const std::map<std::string, std::string>& params)
{
    if (ev)
    {
        NSString* evName = cstr2String(ev);
        NSDictionary* dictParams = evParams2Dictionary(params);
        if (dictParams)
            NSLog(@"FL_logTimedEvent:%@ Params:%@", evName, dictParams);
        
        if (dictParams == nil)
            FL_logTimedEvent(ev);
        else
            [Flurry logEvent:evName withParameters:dictParams timed:YES];
    }
}

void BridgingUtility::FL_endTimedEvent(const char* ev)
{
    EvLogParamKV params;
    FL_endTimedEvent(ev, params);
}

void BridgingUtility::FL_endTimedEvent(const char* ev, const std::map<std::string, std::string>& params)
{
    if (ev)
    {
        NSString* evName = cstr2String(ev);
        NSDictionary* dictParams = evParams2Dictionary(params);
        if (dictParams)
            NSLog(@"FL_endTimedEvent:%@ Params:%@", evName, dictParams);
        else
            NSLog(@"FL_endTimedEvent:%@", evName);
        [Flurry endTimedEvent:evName withParameters:dictParams];
    }
}

#else
void BridgingUtility::FL_startSession(bool reportCrashes) { }
void BridgingUtility::FL_setUserID(const char* userid) { }
void BridgingUtility::FL_logEvent(const char* ev) { }
void BridgingUtility::FL_logEvent(const char* ev, const std::map<std::string, std::string>& params) { }
void BridgingUtility::FL_logTimedEvent(const char* ev) { }
void BridgingUtility::FL_logTimedEvent(const char* ev, const std::map<std::string, std::string>& params) { }
void BridgingUtility::FL_endTimedEvent(const char* ev) { }
void BridgingUtility::FL_endTimedEvent(const char* ev, const std::map<std::string, std::string>& params) { }
#endif

// Crashlytics
#if CRASHLYTICS_ENABLED
void BridgingUtility::CL_start(void)
{
    NSLog(@"BridgingUtility - Starting Crashlytics...");
    [Crashlytics startWithAPIKey:@"7fb7e316cc4cca4d2b3eff74fa989ec709b8e1c5"];
}

void BridgingUtility::CL_initCustomKeys(void)
{
    CL_setStringValue("LevelMenu", CRASH_CONTEXT_GAME_STATE);
    CL_setStringValue("None", CRASH_CONTEXT_LEVEL_NAME);
    CL_setStringValue("None", CRASH_CONTEXT_PUZZLE_NAME);
    CL_setStringValue(Localizer::locale2String(Localizer::getLocale()), CRASH_CONTEXT_LOCALE);
    CL_setStringValue("None", CRASH_CONTEXT_MENU);
    CL_setIntValue(CMSettings::kDefaultSfxVolume, CRASH_CONTEXT_SFX);
    CL_setIntValue(CMSettings::kDefaultMusicVolume, CRASH_CONTEXT_MUSIC);
    CL_setBoolValue(false, CRASH_CONTEXT_ICLOUD);
    CL_setBoolValue(false, CRASH_CONTEXT_COLOR_BLIND);
}

void BridgingUtility::CL_setUserIdentifier(const char* userid)
{
    if (userid)
        [Crashlytics setUserIdentifier:[NSString stringWithCString:userid encoding:NSUTF8StringEncoding]];
}

void BridgingUtility::CL_setUserName(const char* username)
{
    if (username)
        [Crashlytics setUserName:[NSString stringWithCString:username encoding:NSUTF8StringEncoding]];
}

void BridgingUtility::CL_setUserEmail(const char* email)
{
    if (email)
        [Crashlytics setUserEmail:[NSString stringWithCString:email encoding:NSUTF8StringEncoding]];
}

void BridgingUtility::CL_setIntValue(int value, const char* key)
{
    if (key)
        [Crashlytics setIntValue:value forKey:[NSString stringWithCString:key encoding:NSUTF8StringEncoding]];
}

void BridgingUtility::CL_setBoolValue(bool value, const char* key)
{
    if (key)
        [Crashlytics setBoolValue:value forKey:[NSString stringWithCString:key encoding:NSUTF8StringEncoding]];
}

void BridgingUtility::CL_setFloatValue(float value, const char* key)
{
    if (key)
        [Crashlytics setFloatValue:value forKey:[NSString stringWithCString:key encoding:NSUTF8StringEncoding]];
}

void BridgingUtility::CL_setStringValue(const char* value, const char* key)
{
    if (value && key)
        [Crashlytics setObjectValue:[NSString stringWithCString:value encoding:NSUTF8StringEncoding]
                             forKey:[NSString stringWithCString:key encoding:NSUTF8StringEncoding]];
}

void BridgingUtility::CL_log(const char* msg)
{
    if (msg)
        CLS_LOG(@"%@", [NSString stringWithCString:msg encoding:NSUTF8StringEncoding]);
}

void BridgingUtility::CL_crash(void)
{
#if DEBUG
    [[Crashlytics sharedInstance] crash];
#endif
}
#else
void BridgingUtility::CL_start(void) { }
void BridgingUtility::CL_initCustomKeys(void) { }
void BridgingUtility::CL_setUserIdentifier(const char* userid) { }
void BridgingUtility::CL_setUserName(const char* username) { }
void BridgingUtility::CL_setUserEmail(const char* email) { }
void BridgingUtility::CL_setIntValue(int value, const char* key) { }
void BridgingUtility::CL_setBoolValue(bool value, const char* key) { }
void BridgingUtility::CL_setFloatValue(float value, const char* key) { }
void BridgingUtility::CL_setStringValue(const char* value, const char* key) { }
void BridgingUtility::CL_log(const char* msg) { }
void BridgingUtility::CL_crash(void) { }
#endif

// TestFlight
#if TESTFLIGHT_ENABLED
void TF_setDeviceIdentifier(void)
{
//    if ([ASIdentifierManager class])
//    {
//        NSString* uuidString = [[[ASIdentifierManager sharedManager] advertisingIdentifier] UUIDString];
//        if (uuidString == nil)
//            uuidString = @"Bad UUID string";
//        NSLog(@"UUID string: %@", uuidString);
//        [TestFlight setDeviceIdentifier:uuidString];
//    }
//    else
//        [TestFlight setDeviceIdentifier:@"Pre-iOS6 deviceID."];
}

static bool TF_hasLaunched = false;
void BridgingUtility::TF_takeOff(void)
{
    if (TF_hasLaunched)
        return;
    
//    TF_setDeviceIdentifier();
//    [TestFlight setOptions:@{
//                             TFOptionReportCrashes : @NO,
//                             //TFOptionDisableInAppUpdates : @YES
//                             }];
    [TestFlight takeOff:@"936fff0c-bcf9-4ce4-a6a5-df482b5c9768"];
    TF_hasLaunched = true;
    
#if DEBUG
    NSLog(@"DEBUG build");
#else
    NSLog(@"RELEASE build");
#endif
}

void BridgingUtility::TF_addCustomEnvironmentInformation(const char* information, const char* key)
{
    if (information && key)
        [TestFlight addCustomEnvironmentInformation:[NSString stringWithUTF8String:information] forKey:[NSString stringWithUTF8String:key]];
}

void BridgingUtility::TF_passCheckpoint(const char* checkpointName)
{
    if (checkpointName)
    {
        NSString* nameStr = [NSString stringWithCString:SceneController::getUserDefaultString(CMSettings::S_BETA_NAME).c_str() encoding:NSUTF8StringEncoding];
        NSString* cpStr = [NSString stringWithCString:checkpointName encoding:NSUTF8StringEncoding];
        [TestFlight passCheckpoint:[@[nameStr, @": ", cpStr] componentsJoinedByString:@""]];
    }
}

void BridgingUtility::TF_submitFeedback(const char* feedback)
{
    if (feedback)
    {
        // Prepend tester's name to feedback so we can filter.
        NSString* nameStr = [NSString stringWithCString:SceneController::getUserDefaultString(CMSettings::S_BETA_NAME).c_str() encoding:NSUTF8StringEncoding];
        NSString* feedbackStr = [NSString stringWithCString:feedback encoding:NSASCIIStringEncoding];
        [TestFlight submitFeedback:[@[nameStr, @": ", feedbackStr] componentsJoinedByString:@""]];
        //CCLog([[@[nameStr, @": ", feedbackStr] componentsJoinedByString:@""] UTF8String]);
    }
}
#else
void BridgingUtility::TF_takeOff(void) { }
void BridgingUtility::TF_addCustomEnvironmentInformation(const char* information, const char* key) { }
void BridgingUtility::TF_passCheckpoint(const char* checkpointName) { }
void BridgingUtility::TF_submitFeedback(const char* feedback) { }
#endif

