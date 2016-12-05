//
//  CMiTunesManager.m
//  PuzzleWizard
//
//  Created by Paul McPhee on 27/02/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#define RATE_THE_GAME_TESTING 0

#import "CMiTunesManager.h"
#import "AppController.h"
#include <SceneControllers/SceneController.h>
#include <Managers/GameSettings.h>
#include <Puzzle/Controllers/GameProgressController.h>
#include <SceneControllers/GameController.h>
#include <Utils/BridgingUtility.h>
#import <StoreKit/StoreKit.h>

#ifdef __APPLE__
    #ifdef CHEEKY_MOBILE
        #import <UIKit/UIKit.h>
        #if DEBUG && RATE_THE_GAME_TESTING
            #define _AppId @"457446957" // Jetpack Joyride
        #else
            #define _AppId @"796300430" // Puzzle Wizard
        #endif
        #define _ReviewAppURLString_6_0 @"itms-apps://ax.itunes.apple.com/WebObjects/MZStore.woa/wa/viewContentsUserReviews?type=Purple+Software&id="
        #define _ReviewAppURLString @"itms-apps://itunes.apple.com/app/id"

        NSString *const kReviewAppURLString_6_0 = (_ReviewAppURLString_6_0 _AppId);
        NSString *const kReviewAppURLString = (_ReviewAppURLString _AppId);
    #else
        #import <AppKit/AppKit.h>
        #if DEBUG && RATE_THE_GAME_TESTING
            #define _AppId @"698147017" // Reaper
        #else
            #define _AppId @"796300662" // Puzzle Wizard
        #endif
        #define _ReviewAppURLString @"macappstore://itunes.apple.com/app/id"
        // Reaper - test URL: https://itunes.apple.com/app/reaper/id698147017?mt=12

        NSString *const kReviewAppURLString = (_ReviewAppURLString _AppId @"?mt=12");
    #endif
#endif

#ifdef CHEEKY_MOBILE
    #define kAppId 796300430
#else
    #define kAppId 796300662
#endif

#if !DEBUG || !RATE_THE_GAME_TESTING
//static const int kSolvedQuotaBetweenPrompts = 6;
static const int kMinSolvedQuotaBeforePrompting = 12;
#endif

@interface CMiTunesManager()

+ (NSInteger)getAppId;
+ (NSString *)getReviewURL;

@end


@implementation CMiTunesManager

+ (NSInteger)getAppId {
    return kAppId;
}

+ (NSString *)getReviewURL {
#ifdef CHEEKY_MOBILE
    if (BridgingUtility::isIOSFeatureSupported("7.0"))
        return kReviewAppURLString;
    else
        return kReviewAppURLString_6_0;
#else
    return kReviewAppURLString;
#endif
}

+ (BOOL)shouldPromptForRating {
#if CM_BETA
    return NO;
#elif DEBUG && RATE_THE_GAME_TESTING
    return YES;
#else
    int solvedCountAtLastPrompt = SceneController::getUserDefaultInt(CMSettings::I_RATE_GAME, 0);
    int solvedCountThisPrompt = GameProgressController::GPC()->getNumSolvedPuzzles();

    if (solvedCountAtLastPrompt == -1)
        return NO;
    else if (solvedCountThisPrompt >= kMinSolvedQuotaBeforePrompting)
        return YES;
    else
        return NO;
    
//    if (solvedCountAtLastPrompt == -1)
//        return NO;
//    else if (solvedCountThisPrompt > 36)
//        return NO; // Stop bothering them at this point
//    else if (solvedCountThisPrompt >= kMinSolvedQuotaBeforePrompting && solvedCountThisPrompt - solvedCountAtLastPrompt >= kSolvedQuotaBetweenPrompts)
//        return YES;
//    else
//        return NO;
#endif
}

+ (void)userRespondedToRatingPrompt:(BOOL)response {
    if (response)
    {
#ifdef __APPLE__
        #ifdef CHEEKY_MOBILE
            if ([SKStoreProductViewController class] != nil) //if (BridgingUtility::isIOSFeatureSupported("6.0"))
                [AppController showStoreKitProductView:_AppId];
            else
            {
                [[UIApplication sharedApplication] openURL:[NSURL URLWithString:[CMiTunesManager getReviewURL]]];
                [CMiTunesManager userDidRateTheGame:YES];
            }
        #else
            [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[CMiTunesManager getReviewURL]]];
        
        bool didRate = true;
        GameController::GC()->notifyEvent(GameController::EV_TYPE_USER_DID_RATE_THE_GAME(), (void*)&didRate);
        #endif
#endif
    }
    else
    {
        SceneController::setUserDefault(CMSettings::I_RATE_GAME, GameProgressController::GPC()->getNumSolvedPuzzles());
    }
    
    SceneController::flushUserDefaults();
}

+ (void)userDidRateTheGame:(BOOL)success {
    // -1 means don't ask anymore because thet agreed to rate it already.
    SceneController::setUserDefault(CMSettings::I_RATE_GAME, success ? -1 : 0);
    SceneController::flushUserDefaults();
}


@end
