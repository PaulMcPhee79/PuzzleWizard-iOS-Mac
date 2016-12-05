#ifndef __BYTE_TWEENER_H__
#define __BYTE_TWEENER_H__

#include "cocos2d.h"
#include <Interfaces/IAnimatable.h>
#include <Events/EventDispatcher.h>
#include <Utils/Transitions.h>
USING_NS_CC;

class ByteTweener : public CCObject, public IAnimatable
{
public:
	static int EV_TYPE_BYTE_TWEENER_CHANGED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
	static int EV_TYPE_BYTE_TWEENER_COMPLETED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }

	ByteTweener(GLubyte startValue = 0, IEventListener* listener = NULL, TRANS_FUNC transition = CMTransitions::LINEAR);
	virtual ~ByteTweener(void);

	void reset(GLubyte value);
	void reset(GLubyte from, GLubyte to, float duration, float delay = 0.0f);
	void reverse(void);
	void syncWithTweener(ByteTweener* tweener);
	virtual void* getTarget(void) const { return (void*)mListener; }
	virtual void advanceTime(float dt);

	int getTag(void) { return mTag; }
	void setTag(int value) { mTag = value; }
	virtual bool isComplete(void) const { return !isDelaying() && mValue == mTo && mDuration == mTotalDuration; }
	bool isDelaying(void) const { return mDuration < 0; }
	GLubyte getTweenedValue(void) const { return mValue; }
	bool isInverted(void) const { return mInverted; }
	void setInverted(bool value) { mInverted = value; }
	IEventListener* getListener(void) const { return mListener; }
	void setListener(IEventListener* value) { mListener = value; }

private:
	void updateValue(void);

	bool mInverted;
	int mTag;
	GLubyte mFrom;
	GLubyte mTo;
	GLubyte mValue;
	float mDuration;
	float mTotalDuration;
	IEventListener* mListener;
	TRANS_FUNC mTransition;
};
#endif // __BYTE_TWEENER_H__
