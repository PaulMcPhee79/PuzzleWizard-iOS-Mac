#ifndef __LEVEL_OVERLAY_H__
#define __LEVEL_OVERLAY_H__

#include "cocos2d.h"
#include <Prop/Prop.h>
#include <Interfaces/IEventListener.h>
class ByteTweener;
USING_NS_CC;

class LevelOverlay : public Prop, public IEventListener
{
public:
	LevelOverlay(int category);
	virtual ~LevelOverlay(void);
    
	virtual bool init(void);
    virtual void setOpacity(GLubyte value);
    CCRect getContentBounds(void);
    virtual void onEvent(int evType, void* evData);
    virtual void advanceTime(float dt);
    
private:
	enum LevelOverlayState { IDLE, GLOW, FADE };
    
    LevelOverlayState getState(void) const { return mState; }
    void setState(LevelOverlayState value);
    
    int mRuneIndex;
    LevelOverlayState mState;
    std::vector<CCSprite*> mRunes;
    CCSprite* mWizardHat;
    ByteTweener* mRuneTweener;
};

#endif // __LEVEL_OVERLAY_H__
