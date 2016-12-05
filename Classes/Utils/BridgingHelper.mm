
#import "BridgingHelper.h"
#import "CDAudioManager.h"
#import "cocos2d.h"
USING_NS_CC;

@implementation BridgingHelper

- (id)init
{
    if (self = [super init])
    {
        audioCompletionCallback = NULL;
    }
    
    return self;
}

- (void)configureAudioMode
{
//#ifdef CHEEKY_MOBILE
    //[CDAudioManager configure:kAMM_FxPlusMusic];
    [CDAudioManager configure:kAMM_FxPlusMusicIfNoOtherAudio];
//#endif
}

- (void)setAudioMode
{
    //[[CDAudioManager sharedManager] setMode:kAMM_FxPlusMusic];
    //[[CDAudioManager sharedManager] setMode:kAMM_FxPlusMusicIfNoOtherAudio];
    //[[CDAudioManager sharedManager] attemptAudioResart];
}

- (BOOL)attemptAudioRestart
{
//#ifdef CHEEKY_MOBILE
    return [[CDAudioManager sharedManager] attemptAudioRestart];
//#else
//    return YES;
//#endif
}

- (void)applyDefaultResignActiveBehavior
{
//#ifdef CHEEKY_MOBILE
    [[CDAudioManager sharedManager] setResignBehavior:kAMRBStopPlay autoHandle:YES];
    //[[CDAudioManager sharedManager] setResignBehavior:kAMRBDoNothing autoHandle:YES];
//#endif
}

- (void)onBackgroundAudioCompleted
{
    if (audioCompletionCallback)
        audioCompletionCallback();
}

- (void)registerAudioCompletionCallback:(P_AUDIO_DID_COMPLETE)callback
{
    if (callback)
        [[CDAudioManager sharedManager] setBackgroundMusicCompletionListener:self selector:@selector(onBackgroundAudioCompleted)];
    else
        [[CDAudioManager sharedManager] setBackgroundMusicCompletionListener:nil selector:nil];
    audioCompletionCallback = callback;
}

@end
