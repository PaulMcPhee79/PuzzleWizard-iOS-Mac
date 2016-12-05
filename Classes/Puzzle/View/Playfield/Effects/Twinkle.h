#ifndef __TWINKLE_H__
#define __TWINKLE_H__

#include "cocos2d.h"
#include <Prop/CMSprite.h>
#include <Events/EventDispatcher.h>
#include <Interfaces/IReusable.h>
#include <Interfaces/IEventListener.h>
class ReusableCache;
class FloatTweener;
class ByteTweener;
USING_NS_CC;

class Twinkle : public CMSprite, public IEventListener, public IReusable
{
public:
	static int EV_TYPE_ANIMATION_COMPLETED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }

	Twinkle(const CCPoint& origin);
	virtual ~Twinkle(void);

	static Twinkle* create(const CCPoint& origin, bool autorelease = true);
	virtual bool init(void);

	void animate(float duration);
	void fastForward(float duration);
	void stopAnimating(void);
	virtual void onEvent(int evType, void* evData);

	void syncWithTwinkle(Twinkle* twinkle);

	static Twinkle* getTwinkle(const CCPoint& origin);
	static void setupReusables(void);
    virtual void returnToPool(void);

	IEventListener* getListener(void) const { return mListener; }
	void setListener(IEventListener* value) { mListener = value; }

	virtual bool isComplete(void) const { return !isAnimating(); }
	virtual void* getTarget(void) const { return (void*)this; }
	virtual void advanceTime(float dt);

	bool isAnimating(void) const { return mIsAnimating; }

private:
    virtual int getReuseKey(void) { return 0; }
	virtual void reuse(void);
    virtual void hibernate(void);
	static IReusable* checkoutReusable(uint reuseKey);
	static void checkinReusable(IReusable* reusable);
	void reverseAnimation(float duration);
	void setAnimating(bool value);

	static ReusableCache* sCache;
    static bool sCaching;

	bool mIsAnimating;
	float mDuration;
	float mDurationPassed;
	int mTweenerCounters[3];
	FloatTweener* mRotateTweener;
	FloatTweener* mScaleTweener;
	ByteTweener* mOpacityTweener;
	IEventListener* mListener;
};
#endif // __TWINKLE_H__
