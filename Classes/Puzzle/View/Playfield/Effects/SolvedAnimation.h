#ifndef __SOLVED_ANIMATION_H__
#define __SOLVED_ANIMATION_H__

#include "cocos2d.h"
#include <Prop/Prop.h>
#include <Events/EventDispatcher.h>
#include <Interfaces/IEventListener.h>
class Twinkle;
class ByteTweener;
class FloatTweener;
USING_NS_CC;

class SolvedAnimation : public Prop, public IEventListener
{
public:
	static int EV_TYPE_ANIMATION_COMPLETED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }

	SolvedAnimation(int category, IEventListener* listener = NULL);
	virtual ~SolvedAnimation(void);

	static SolvedAnimation* create(int category, IEventListener* listener = NULL, bool autorelease = true);
	virtual bool init(void);

	void animate(const CCPoint& from, const CCPoint& to, float duration, float delay = 0.0f);
	void stopAnimating(void);
	virtual void onEvent(int evType, void* evData);

	bool isAnimating(void) const { return mIsAnimating; }
	IEventListener* getListener(void) const { return mListener; }
	void setListener(IEventListener* value) { mListener = value; }

private:
	static const int kNumAnimTweeners = 4;

	void fadeOverTime(float duration, float delay);
	void setAnimating(bool value) { mIsAnimating = value; }

	bool mIsAnimating;
	Twinkle* mTwinkle;
    CCNode* mTwinkleScaler;
	CCSprite* mKey;
	ByteTweener* mFadeTweener;
	FloatTweener* mAnimTweeners[kNumAnimTweeners];

	IEventListener* mListener;
};
#endif // __SOLVED_ANIMATION_H__
