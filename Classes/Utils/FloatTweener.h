#ifndef __FLOAT_TWEENER_H__
#define __FLOAT_TWEENER_H__

#include "cocos2d.h"
#include <Interfaces/IAnimatable.h>
#include <Events/EventDispatcher.h>
#include <Utils/Transitions.h>
class IEventListener;
USING_NS_CC;

class FloatTweener : public CCObject, public IAnimatable
{
public:
	static int EV_TYPE_FLOAT_TWEENER_CHANGED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
	static int EV_TYPE_FLOAT_TWEENER_COMPLETED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }

	FloatTweener(float startValue = 0, IEventListener* listener = NULL, TRANS_FUNC transition = CMTransitions::LINEAR);
	virtual ~FloatTweener(void);

    void forceCompletion(void);
	void reset(float value);
	void reset(float from, float to, float duration, float delay = 0.0f);
	void syncWithTweener(FloatTweener* tweener);
	virtual void* getTarget(void) const { return mListener; }
	virtual void advanceTime(float dt);

	int getTag(void) { return mTag; }
	void setTag(int value) { mTag = value; }
	virtual bool isComplete(void) const
    {
        if (mOverflows)
            return !isDelaying() && mValue >= mTo && mDuration >= mTotalDuration;
        else
            return !isDelaying() && mValue == mTo && mDuration == mTotalDuration;
    }
	bool isDelaying(void) const { return mDuration < 0; }
	float getTweenedValue(void) const { return mValue; }
	float getFromValue(void) const { return mFrom; }
    float getToValue(void) const { return mTo; }
	float getDeltaValue(void) const { return mValue - mPrevValue; }
	float getTotalDeltaValue(void) const { return mValue - mFrom; }
	float getPercentComplete(void) const { return mTotalDuration != 0 ? mDuration / mTotalDuration : 1.0f; }
    bool getOverflows(void) const { return mOverflows; }
	void setOverflows(bool value) { mOverflows = value; }
	bool isInverted(void) const { return mInverted; }
	void setInverted(bool value) { mInverted = value; }
    TRANS_FUNC getTransition(void) const { return mTransition; }
	void setTransition(TRANS_FUNC value);
	IEventListener* getListener(void) const { return mListener; }
	void setListener(IEventListener* value) { mListener = value; }

private:
	void updateValue(void);
	void setValue(float value);

    bool mOverflows;
	bool mInverted;
	int mTag;
	float mFrom;
	float mTo;
	float mValue;
	float mPrevValue;
	float mDuration;
	float mTotalDuration;
	IEventListener* mListener;
	TRANS_FUNC mTransition;
};
#endif // __FLOAT_TWEENER_H__
