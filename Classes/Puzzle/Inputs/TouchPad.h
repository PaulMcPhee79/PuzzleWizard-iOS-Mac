#ifndef __TOUCH_PAD_H__
#define __TOUCH_PAD_H__

#include "cocos2d.h"
#include <Events/EventDispatcher.h>
#include <Interfaces/IEventListener.h>
USING_NS_CC;

class TouchPad : public CCNode, public CCTargetedTouchDelegate, public EventDispatcher
{
public:
	static int EV_TYPE_TOUCH_BEGAN() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
	static int EV_TYPE_TOUCH_MOVED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
	static int EV_TYPE_TOUCH_ENDED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
	static int EV_TYPE_TOUCH_CANCELLED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }

	TouchPad(const CCRect& touchBounds, IEventListener* listener = NULL);
	virtual ~TouchPad(void);

	virtual void onEnter();
    virtual void onExit();
	virtual bool ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent);
	virtual void ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent);
	virtual void ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent);
	virtual void ccTouchCancelled(CCTouch *pTouch, CCEvent *pEvent);

	bool isTouchEnabled(void) const { return mTouchEnabled; }
	void setTouchEnabled(bool value) { mTouchEnabled = value; }
	const CCRect& getTouchBounds(void) const { return mTouchBounds; }
	void setTouchBounds(const CCRect& value) { mTouchBounds = value; }
	IEventListener* getListener(void) const { return mListener; }
	void setListener(IEventListener* value) { mListener = value; }

	const CCPoint& getTouchPos(void) const { return mTouchPos; }
	const CCPoint& getPrevTouchPos(void) const { return mPrevTouchPos; }
	float getTouchDeltaX(void) const { return mTouchPos.x - mPrevTouchPos.x; }
	float getTouchDeltaY(void) const { return mTouchPos.y - mPrevTouchPos.y; }

	void resetTouches(void);
protected:
	bool shouldReportTouch(CCTouch *pTouch);
	void reportTouch(int evType);
	bool containsTouchLocation(CCTouch* pTouch, CCPoint& convertedPoint);
	
	CCRect mTouchBounds;

private:
	bool mTouchEnabled;
	bool mMultiTouch;
	int mTouchID;
	CCPoint mTouchPos;
	CCPoint mPrevTouchPos;
	IEventListener* mListener;
};
#endif // __TOUCH_PAD_H__
