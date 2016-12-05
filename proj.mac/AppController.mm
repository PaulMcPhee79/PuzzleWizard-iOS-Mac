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

#import "AppController.h"
#import "AppDelegate.h"
#import "EAGLView.h"
#import <CCWindow.h>
#import <Foundation/Foundation.h>
#import <Carbon/Carbon.h>
#include <SceneControllers/GameController.h>
#include <Managers/ControlsManager.h>
#include <Managers/GameSettings.h>
#include <Utils/BridgingUtility.h>

static AppDelegate s_sharedApplication;

@implementation AppController

@synthesize window, glView;

-(void) applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // HockeyApp
    [[BITHockeyManager sharedHockeyManager] configureWithIdentifier:@"7108c16089c664c746b6e361f51f0a0b" delegate:self];
    [[BITHockeyManager sharedHockeyManager].crashManager setAutoSubmitCrashReport:YES];
#if DEBUG
    [[BITHockeyManager sharedHockeyManager] setDebugLogEnabled:YES];
#endif
    [[BITHockeyManager sharedHockeyManager] startManager];
    [[BITHockeyManager sharedHockeyManager] testIdentifier];
    
    isUserDefaultsDirty = NO;
    int winWidth = CCUserDefault::sharedUserDefault()->getIntegerForKey(CMSettings::I_WIN_WIDTH, CMSettings::kDefaultWinWidth);
    int winHeight = CCUserDefault::sharedUserDefault()->getIntegerForKey(CMSettings::I_WIN_HEIGHT, CMSettings::kDefaultWinHeight);
    
    // Restrict size to current screen resolution
    NSUInteger styleMask = NSClosableWindowMask | NSResizableWindowMask | NSMiniaturizableWindowMask | NSTitledWindowMask;
    NSRect winFrame = [NSWindow contentRectForFrameRect:NSMakeRect(0, 0, winWidth, winHeight) styleMask:styleMask];
    
    lastWinSize.width = winFrame.size.width;
    lastWinSize.height = winFrame.size.height;
    
#if 1
    NSRect rect = winFrame;
    window = [[NSWindow alloc] initWithContentRect:rect
                                         styleMask:styleMask
                                           backing:NSBackingStoreBuffered
                                             defer:YES];
    [window setContentMinSize:NSMakeSize(100, 56)]; // 16:9
#else
    NSRect rect = NSMakeRect(0, 0, winWidth, winHeight);
    window = [[CCWindow alloc] initWithFrame:rect fullscreen:true];
#endif
    
    // allocate our GL view
    // (isn't there already a shared EAGLView?)
    glView = [[EAGLView alloc] initWithFrame:rect];
    
    // set window parameters
    [window becomeFirstResponder];
    [window setContentView:glView];
    //[window setTitle:@"Puzzle Wizard"];
    [window makeKeyAndOrderFront:self];
    [window setAcceptsMouseMovedEvents:YES];
    [window center];
    
    //[glView setFrameZoomFactor:0.4];
    
    cursorTimeoutCounter = 4;
    cursorTimer = [[NSTimer scheduledTimerWithTimeInterval:0.5 target:self selector:@selector(onMouseActivityTimedOut:) userInfo:nil repeats:YES] retain];
    [[CCEventDispatcher sharedDispatcher] addMouseDelegate:self priority:0];
    [[CCEventDispatcher sharedDispatcher] addKeyboardDelegate:self priority:0];
    cocos2d::CCApplication::sharedApplication()->run();
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onWindowResized:) name:NSWindowDidResizeNotification object:window];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onWindowWillClose:) name:NSWindowWillCloseNotification object:window];
}

- (void)crashManagerWillSendCrashReport:(BITCrashManager *)crashManager {
    NSLog(@"CrashManager: Will send crash report...");
}

- (void)crashManager:(BITCrashManager *)crashManager didFailWithError:(NSError *)error {
    NSLog(@"CrashManager: Failed to send crash report...");
    NSLog(@"%@", error.localizedDescription);
}

- (void)crashManagerDidFinishSendingCrashReport:(BITCrashManager *)crashManager {
    NSLog(@"CrashManager: Successfully sent crash report...");
}

-(void)applicationDidBecomeActive:(NSNotification *)notification
{
    if (BridgingUtility::isFullscreen())
    {
        CCDirector::sharedDirector()->resume();
        CCDirector::sharedDirector()->startAnimation();
    }
    
    GameController::GC()->applicationWillEnterForeground();
    [NSCursor setHiddenUntilMouseMoves:YES];
}

-(void)applicationWillResignActive:(NSNotification *)notification
{
    if (BridgingUtility::isFullscreen())
    {
        CCDirector::sharedDirector()->stopAnimation();
        CCDirector::sharedDirector()->pause();
    }
    
    GameController::GC()->applicationDidEnterBackground();
}

-(BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)theApplication
{
    return YES;
}

- (void)applicationDidChangeScreenParameters:(NSNotification *)notification
{
    [AppController resolutionDidChange];
}

+ (void) resolutionDidChange
{
    NSWindow* currentWindow = [[EAGLView sharedEGLView] getCurrentWindow];
    if (currentWindow)
    {
        NSSize winSize = [currentWindow frame].size;
        NSRect contentRect = [currentWindow contentRectForFrameRect:NSMakeRect(0, 0, winSize.width, winSize.height)];
        
        CCEGLView* pEGLView = CCEGLView::sharedOpenGLView();
        CCSize viewSize = pEGLView->getFrameSize();
        
        if ((int)viewSize.width != (int)contentRect.size.width || (int)viewSize.height != (int)contentRect.size.height)
        {
            pEGLView->setFrameSize(contentRect.size.width, contentRect.size.height);
            pEGLView->setDesignResolutionSize(contentRect.size.width, contentRect.size.height, kResolutionNoBorder);
            GameController::GC()->resolutionDidChange((int)contentRect.size.width, (int)contentRect.size.height);
        }
    }
}

- (void) flushUserDefaults
{
    if (isUserDefaultsDirty)
    {
        isUserDefaultsDirty = NO;
        
        CCUserDefault::sharedUserDefault()->setBoolForKey(CMSettings::B_FULLSCREEN, BridgingUtility::isFullscreen());
        CCUserDefault::sharedUserDefault()->setIntegerForKey(CMSettings::I_WIN_WIDTH, (int)lastWinSize.width);
        CCUserDefault::sharedUserDefault()->setIntegerForKey(CMSettings::I_WIN_HEIGHT, (int)lastWinSize.height);
        CCUserDefault::sharedUserDefault()->flush();
    }
}

-(void) dealloc
{
    [cursorTimer invalidate], [cursorTimer release], cursorTimer = nil;
    [[CCEventDispatcher sharedDispatcher] removeMouseDelegate:self];        // These retain us, we can't be in dealloc while delegates
    [[CCEventDispatcher sharedDispatcher] removeKeyboardDelegate:self];     // exist. Does it matter in AppController?
    cocos2d::CCDirector::sharedDirector()->end();
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [super dealloc];
}

#pragma mark -
#pragma mark IB Actions

-(IBAction) toggleFullScreen:(id)sender
{
    isUserDefaultsDirty = YES;
	EAGLView* pView = [EAGLView sharedEGLView];
	[pView setFullScreen:!pView.isFullScreen];
    [AppController resolutionDidChange];
    
    [[CCEventDispatcher sharedDispatcher] removeMouseDelegate:self];
    [[CCEventDispatcher sharedDispatcher] removeKeyboardDelegate:self];
    [[CCEventDispatcher sharedDispatcher] addMouseDelegate:self priority:0];
    [[CCEventDispatcher sharedDispatcher] addKeyboardDelegate:self priority:0];
}

-(IBAction) exitFullScreen:(id)sender
{
    isUserDefaultsDirty = YES;
	[[EAGLView sharedEGLView] setFullScreen:NO];
    [AppController resolutionDidChange];
    
    [[CCEventDispatcher sharedDispatcher] removeMouseDelegate:self];
    [[CCEventDispatcher sharedDispatcher] removeKeyboardDelegate:self];
    [[CCEventDispatcher sharedDispatcher] addMouseDelegate:self priority:0];
    [[CCEventDispatcher sharedDispatcher] addKeyboardDelegate:self priority:0];
}

- (void) onWindowResized:(NSNotification *)notification
{
    isUserDefaultsDirty = YES;
    lastWinSize = [window frame].size;
    [AppController resolutionDidChange];
}

- (void) onWindowWillClose:(NSNotification *)notification
{
    GameController::GC()->applicationWillTerminate();
}

//@protocol CCKeyboardEventDelegate <NSObject>
-(BOOL) ccKeyUp:(NSEvent*)event
{
    ControlsManager::CM()->keyUp(event.keyCode);
    return YES;
}

-(BOOL) ccKeyDown:(NSEvent*)event
{
    ControlsManager::CM()->keyDown(event.keyCode);
    return YES;
}

-(BOOL) ccFlagsChanged:(NSEvent*)event
{
    //ControlsManager::CM()->keyDown(event.modifierFlags); // NSUInteger
    return YES;
}

// @protocol CCMouseEventDelegate <NSObject>

- (void) onMouseActivityDetected
{
    cursorTimeoutCounter = 0;
    [NSCursor setHiddenUntilMouseMoves:NO];
}

- (void) onMouseActivityTimedOut:(NSTimer*)theTimer
{
    if (GameController::GC()->shouldAppExit())
    {
        GameController::GC()->applicationWillTerminate();
        [NSApp terminate:self];
    }
    else
    {
        [self flushUserDefaults];
        
        if (++cursorTimeoutCounter == 5)
            [NSCursor setHiddenUntilMouseMoves:YES];
    }
}

-(BOOL) ccMouseDown:(NSEvent*)event { [self onMouseActivityDetected]; return true; }

-(BOOL) ccMouseDragged:(NSEvent*)event { [self onMouseActivityDetected]; return true; }

-(BOOL) ccMouseMoved:(NSEvent*)event { [self onMouseActivityDetected]; return true; }

-(BOOL) ccMouseUp:(NSEvent*)event { [self onMouseActivityDetected]; return true; }

-(BOOL) ccRightMouseDown:(NSEvent*)event { [self onMouseActivityDetected]; return true; }

-(BOOL) ccRightMouseDragged:(NSEvent*)event { [self onMouseActivityDetected]; return true; }

-(BOOL) ccRightMouseUp:(NSEvent*)event { [self onMouseActivityDetected]; return true; }

-(BOOL) ccOtherMouseDown:(NSEvent*)event { [self onMouseActivityDetected]; return true; }

-(BOOL) ccOtherMouseDragged:(NSEvent*)event { [self onMouseActivityDetected]; return true; }

-(BOOL) ccOtherMouseUp:(NSEvent*)event { [self onMouseActivityDetected]; return true; }

- (BOOL)ccScrollWheel:(NSEvent *)theEvent { [self onMouseActivityDetected]; return true; }

- (void)ccMouseEntered:(NSEvent *)theEvent { [self onMouseActivityDetected]; }

- (void)ccMouseExited:(NSEvent *)theEvent { [self onMouseActivityDetected]; }

@end
