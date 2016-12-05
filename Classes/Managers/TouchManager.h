#ifndef __TOUCH_MANAGER_H__
#define __TOUCH_MANAGER_H__

#include "cocos2d.h"
#include <Prop/Prop.h>
#include <Interfaces/IEventListener.h>
#include <Managers/TouchTypes.h>
class TouchPad;
USING_NS_CC;

class TouchManager : public Prop, public IEventListener
{
public:
	TouchManager(int category, const CCRect& touchBounds);
	virtual ~TouchManager(void);
    
    bool isEnabled(void) { return mEnabled; }
    void enable(bool enable) { mEnabled = enable; }

    void setTouchBounds(const CCRect& value);
    
    float getTimeSinceLastTouch(void) const { return mTimeSinceLastTouch; }
    void setTimeSinceLastTouch(float value) { mTimeSinceLastTouch = value; }
    
	void resetTouches(void);
	void registerForTouches(CMTouches::TouchCategory category, IEventListener* listener);
	void deregisterForTouches(CMTouches::TouchCategory category, IEventListener* listener);

	virtual void onEvent(int evType, void* evData);

private:
	void dispatchTouchNotices(int evType);

    bool mEnabled;
	bool mIsLocked;
    float mTimeSinceLastTouch;
	CMTouches::TouchNotice mTouchNotice;
	std::vector<IEventListener*> mTouchListeners; // Each TouchCategory is exclusive per TouchManager, so don't need a list of lists.
	std::map<int, IEventListener*> mAddQueue;
	std::map<int, IEventListener*> mRemoveQueue;
	TouchPad* mTouchPad;
};
#endif // __TOUCH_MANAGER_H__
