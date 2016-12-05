#ifndef __GAME_CONTROLLER_H__
#define __GAME_CONTROLLER_H__

#include "cocos2d.h"
#include <Events/EventDispatcher.h>
#include "SceneController.h"
USING_NS_CC;

class ProgressBar;
class ByteTweener;
class FloatTweener;

class GameController : public CCObject, public IEventListener
{
public:
    static int EV_TYPE_USER_DID_RATE_THE_GAME() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
    
    static const char* CM_NOTICE_GC_CONTENT_LOAD_BEGAN;
    static const char* CM_NOTICE_GC_CONTENT_LOAD_PROGRESSED;
    static const char* CM_NOTICE_GC_CONTENT_LOAD_COMPLETED;
    
	static GameController* GC(void);
	virtual ~GameController(void);
	
	void update(float dt);

	int getUnqiueKey(void) { return mUniqueKey++; }

    bool isHighPerformanceMachine(void) const;
    double getFps(void) const { return mFps; }
    void setFps(double value) { mFps = value; }
    float getContentLoadProgress(void);
    float getContentLoadDuration(void) const { return mContentLoadDuration; }
    static float getMaximizingScale(void);
    
    void resolutionDidChange(int width, int height);
    void applicationDidEnterBackground(void);
    void applicationWillEnterForeground(void);
    void applicationWillTerminate(void);
    void splashViewDidHide(void);
    void invalidateStateCaches(void);
    CCNode* createSplashNode(void);
    int getSplashTagForKey(const char* key);
    void destroySplashNodeResources(void);
    CCSprite* createBgQuad(void);
    ProgressBar* createProgressBar(void);
    
    void registerForEvent(int evType, IEventListener* listener);
    void deregisterForEvent(int evType, IEventListener* listener);
    void notifyEvent(int evType, void* evData);
    
    virtual void onEvent(int evType, void* evData);
    
    bool shouldAppExit(void) const { return mAppShouldExit; }
    void exitApp(void);
    
private:
	GameController(void);
	GameController(const GameController& other);
	GameController& operator=(const GameController& rhs);
	void init(void);
	static CCSize getAdjustedWinSize(void); // For testing splash screen layout on Android
    static CCSize getProgressBarDimensions(void);
	
    bool mAppShouldExit;
    bool mDidAppTerminate;
    bool mSplashPaused;
	int mUniqueKey;
    double mFps;
    EventDispatcher mEventDispatcher;
	SceneController* mCurrentScene;
    
    float mContentLoadDuration;
    CCSprite* mBgQuad;
    ProgressBar* mProgressBar;
    ByteTweener* mSplashTweener;
    CCSprite* mSplashMascot;
    CCNode* mSplashNode;
    FloatTweener** mSplashAnimators;
    FloatTweener* mSplashAnimatorsBacking[2];
    CCScene* mSplashScene;
};
#endif // __GAMECONTROLLER_H__
