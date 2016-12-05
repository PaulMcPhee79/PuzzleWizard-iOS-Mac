/****************************************************************************
 Copyright (c) 2010 cocos2d-x.org
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/
#import <HockeySDK/HockeySDK.h>
#import <UIKit/UIKit.h>
#import "AppController.h"
#import "cocos2d.h"
#import "EAGLView.h"
#import "AppDelegate.h"
#import "DeviceDetails.h"
#include <SceneControllers/GameController.h>
#include <Utils/BridgingUtility.h>
#include "../../../../CocosDenshion/include/SimpleAudioEngine.h"

#import "RootViewController.h"

static RootViewController *g_rootVC = nil;

@implementation AppController

#pragma mark -
#pragma mark Application lifecycle

+ (RootViewController *)getRootVC
{
    return g_rootVC;
}

// cocos2d application instance
static AppDelegate s_sharedApplication;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // HockeyApp
    [[BITHockeyManager sharedHockeyManager] configureWithIdentifier:@"4667afb4789e998a102caf001ad4f3be"];
    [[BITHockeyManager sharedHockeyManager] startManager];
    [[BITHockeyManager sharedHockeyManager].authenticator authenticateInstallation];
    [[BITHockeyManager sharedHockeyManager] testIdentifier];
    

    // Add the view controller's view to the window and display.
    window = [[UIWindow alloc] initWithFrame: [[UIScreen mainScreen] bounds]];
    EAGLView *__glView = [EAGLView viewWithFrame: [window bounds]
                                        pixelFormat: kEAGLColorFormatRGBA8
                                        depthFormat: 0 //GL_DEPTH_COMPONENT16
                                 preserveBackbuffer: NO
                                      sharegroup:nil
                                   multiSampling:NO
                                 numberOfSamples:0];
    
    // Use RootViewController manage EAGLView 
    viewController = [[RootViewController alloc] initWithNibName:nil bundle:nil];
    //viewController.wantsFullScreenLayout = YES; // Not needed iOS7+
    viewController.view = __glView;
    
    if (g_rootVC == nil)
        g_rootVC = viewController;

    // Set RootViewController to window
    if ([[UIDevice currentDevice].systemVersion floatValue] < 6.0)
        [window addSubview:viewController.view]; // warning: addSubView unnecessary on iOS6
    
    [window setRootViewController:viewController];
    [window makeKeyAndVisible];

    [[UIApplication sharedApplication] setStatusBarHidden: YES];
    [[UIApplication sharedApplication] setIdleTimerDisabled:YES];
    
    cocos2d::CCApplication::sharedApplication()->run();
    
    // Splash fade-in
    std::string splashPath = BridgingUtility::getSplashImagePath();
    
    if (!splashPath.empty() && [UIScreen mainScreen])
    {
        NSString* nsSplashPath = [NSString stringWithUTF8String: splashPath.c_str()];
        UIImage *splashImage = [UIImage imageWithContentsOfFile:
                                [[NSBundle mainBundle] pathForResource:nsSplashPath ofType:@"png"]];
        if (splashImage)
        {
            splashView = [[[UIImageView alloc] initWithImage:splashImage] autorelease];
            [[viewController view] addSubview:splashView];
            
            float deg2rad = -90 / 180.0f * 3.14159265359f;
            CGRect screenBounds = [[UIScreen mainScreen] bounds];
            splashView.center = CGPointMake(screenBounds.size.height / 2, screenBounds.size.width / 2); // Landscape, so reverse axes
            splashView.transform = CGAffineTransformMakeRotation(deg2rad);
            
//            deg2rad = 90 / 180.0f * 3.14159265359f;
//            progressView = [[[UIProgressView alloc] initWithProgressViewStyle:UIProgressViewStyleDefault] autorelease];
//            float progWidth = screenBounds.size.height / 2, progHeight = progressView.frame.size.height;
//            progressView.frame = CGRectMake(-progWidth / 2, -progHeight / 2, progWidth, progHeight);
//            progressView.transform = CGAffineTransformMakeRotation(deg2rad);
//            progressView.center = CGPointMake(progHeight / 2 + screenBounds.size.width / 20, screenBounds.size.height / 2);  // Landscape, so reverse axes
//            progressView.progressTintColor = [UIColor colorWithRed:130 / 255.0f green:20 / 255.0f blue:210 / 255.0f alpha:1.0f]; // Purple
//            progressView.progress = 0;
//            [splashView addSubview:progressView];
            
            contentLoaded = NO;
            [[NSNotificationCenter defaultCenter] addObserver:self
                                                     selector:@selector(onGameControllerContentLoadBegan:)
                                                         name:[NSString stringWithCString:GameController::CM_NOTICE_GC_CONTENT_LOAD_BEGAN encoding:NSUTF8StringEncoding]
                                                       object:nil];
            
            [[NSNotificationCenter defaultCenter] addObserver:self
                                                     selector:@selector(onGameControllerContentLoadProgressed:)
                                                         name:[NSString stringWithCString:GameController::CM_NOTICE_GC_CONTENT_LOAD_PROGRESSED encoding:NSUTF8StringEncoding]
                                                       object:nil];
            
            [[NSNotificationCenter defaultCenter] addObserver:self
                                                     selector:@selector(onGameControllerContentLoadCompleted:)
                                                         name:[NSString stringWithCString:GameController::CM_NOTICE_GC_CONTENT_LOAD_COMPLETED encoding:NSUTF8StringEncoding]
                                                       object:nil];
        }
    }
    
    return YES;
}

- (void)onGameControllerContentLoadBegan:(NSNotification* )notification {
    if (splashView)
        [UIView animateWithDuration:0.25
                              delay:0.0
                            options:UIViewAnimationCurveLinear
                         animations:^{splashView.alpha = 0;}
                         completion:^(BOOL finished) {
                             [splashView removeFromSuperview];
                             splashView = nil;
                             GameController::GC()->splashViewDidHide();
                         }];
}

- (void)onGameControllerContentLoadProgressed:(NSNotification* )notification {
//    if (progressView)
//        progressView.progress = GameController::GC()->getContentLoadProgress();
}

- (void)onGameControllerContentLoadCompleted:(NSNotification* )notification {
    contentLoaded = YES;
    
//    if (contentLoaded)
//        return;
//    contentLoaded = YES;
//    
//    if (splashView)
//        [UIView animateWithDuration:1.0
//                              delay:0.25
//                            options:UIViewAnimationCurveLinear
//                         animations:^{splashView.alpha = 0;}
//                         completion:^(BOOL finished) {
//                             [splashView removeFromSuperview];
//                             splashView = nil;
//                             GameController::GC()->splashViewDidHide();
//                         }];
//    if (progressView)
//        [UIView animateWithDuration:0.25
//                              delay:0
//                            options:UIViewAnimationCurveLinear
//                         animations:^{progressView.alpha = 0;}
//                         completion:^(BOOL finished) {
//                             [progressView removeFromSuperview];
//                             progressView = nil;
//                         }];
}

+ (void)showStoreKitProductView:(NSString *)appId
{
    if ([AppController getRootVC])
        [[AppController getRootVC] showStoreKitProductView:appId];
}

- (void)applicationWillResignActive:(UIApplication *)application {
    /*
     Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
     Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
     */
    //cocos2d::CCDirector::sharedDirector()->pause();
    BridgingUtility::pauseEngine();
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
    /*
     Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
     */
    //cocos2d::CCDirector::sharedDirector()->resume();
    BridgingUtility::resumeEngine();
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    /*
     Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later. 
     If your application supports background execution, called instead of applicationWillTerminate: when the user quits.
     */
    cocos2d::CCApplication::sharedApplication()->applicationDidEnterBackground();
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
    /*
     Called as part of  transition from the background to the inactive state: here you can undo many of the changes made on entering the background.
     */
    cocos2d::CCApplication::sharedApplication()->applicationWillEnterForeground();
}

- (void)applicationWillTerminate:(UIApplication *)application {
    /*
     Called when the application is about to terminate.
     See also applicationDidEnterBackground:.
     */
    CocosDenshion::SimpleAudioEngine::end();
}


#pragma mark -
#pragma mark Memory management

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application {
    /*
     Free up as much memory as possible by purging cached data objects that can be recreated (or reloaded from disk) later.
     */
}


- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [super dealloc];
}


@end

