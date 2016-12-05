
typedef void (*P_AUDIO_DID_COMPLETE)(void);

@interface BridgingHelper : NSObject
{
    P_AUDIO_DID_COMPLETE audioCompletionCallback;
}

- (void)configureAudioMode;
- (void)setAudioMode;
- (BOOL)attemptAudioRestart;
- (void)applyDefaultResignActiveBehavior;
- (void)registerAudioCompletionCallback:(P_AUDIO_DID_COMPLETE)callback;
- (void)onBackgroundAudioCompleted;

@end
