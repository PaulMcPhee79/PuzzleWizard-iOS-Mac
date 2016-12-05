#ifndef __INT_TWEENER_H__
#define __INT_TWEENER_H__

#include "cocos2d.h"
#include <Interfaces/IAnimatable.h>
#include <Events/EventDispatcher.h>
#include <Utils/Transitions.h>
USING_NS_CC;

class IntTweener : public CCObject, public IAnimatable
{
public:
	static int EV_TYPE_INT_TWEENER_CHANGED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
	static int EV_TYPE_INT_TWEENER_COMPLETED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }

	IntTweener(int startValue = 0, IEventListener* listener = NULL, TRANS_FUNC transition = CMTransitions::LINEAR);
	virtual ~IntTweener(void);

	void reset(int value);
	void reset(int from, int to, float duration, float delay = 0.0f);
	virtual void* getTarget(void) const { return mListener; }
	virtual void advanceTime(float dt);

	int getTag(void) { return mTag; }
	void setTag(int value) { mTag = value; }
	virtual bool isComplete(void) const { return !isDelaying() && mValue == mTo && mDuration == mTotalDuration; }
	bool isDelaying(void) const { return mDuration < 0; }
	int getTweenedValue(void) const { return mValue; }
	bool isInverted(void) const { return mInverted; }
	void setInverted(bool value) { mInverted = value; }
	IEventListener* getListener(void) const { return mListener; }
	void setListener(IEventListener* value) { mListener = value; }

private:
	bool mInverted;
	int mTag;
	int mFrom;
	int mTo;
	int mValue;
	float mDuration;
	float mTotalDuration;
	IEventListener* mListener;
	TRANS_FUNC mTransition;
};
#endif // __INT_TWEENER_H__
