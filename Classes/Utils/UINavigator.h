#ifndef __UINAVIGATOR_H__
#define __UINAVIGATOR_H__

#include "cocos2d.h"
#include <Interfaces/INavigable.h>
USING_NS_CC;

class UINavigator : public CCObject, public INavigable
{
public:
	UINavigator(NavigationMap navMap);
	virtual ~UINavigator(void);
    
    bool doesRepeat(void) const { return mRepeats; }
    void setRepeats(bool value) { mRepeats = value; }
    float getRepeatDelay(void) const { return mRepeatDelay; }
    void setRepeatDelay(float value) { mRepeatDelay = MAX(0.1f, value); }
    int getNavIndex(void) const { return mNavIndex; }
    int getNavCount(void) const { return (int)mNavigables.size(); }
    
    void addNav(CCNode* nav);
    void removeNav(CCNode* nav);
    void activateNextActiveNav(int dir);
    void update(int controllerState);
    void advanceTime(float dt);
    
    CCNode* getCurrentNav(void) const { return getNavIndex() < getNavCount() ? mNavigables[getNavIndex()] : NULL; }
    virtual void resetNav(void);
    virtual void movePrevNav(void);
    virtual void moveNextNav(void);
    
private:    
    void activateNav(CCNode* nav);
    void deactivateNav(CCNode* nav);
    int getRepeatDir(void) const { return mRepeatDir; }
    void setRepeatDir(int value)
    {
        if (mRepeatDir != value)
        {
            mRepeatDir = value;
            mRepeatCounter = 2 * mRepeatDelay;
        }
    }
    
    bool mRepeats;
    int mRepeatDir;
    float mRepeatCounter;
    float mRepeatDelay;
    
    int mNavIndex;
    std::vector<CCNode*> mNavigables;
};
#endif // __UINAVIGATOR_H__
