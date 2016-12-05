#ifndef __FPS_SAMPLER_H__
#define __FPS_SAMPLER_H__

#include "cocos2d.h"
#include <Prop/Prop.h>
#include <Events/EventDispatcher.h>
USING_NS_CC;

class FpsSampler : public Prop
{
public:
	static int EV_TYPE_FPS_SAMPLER_COMPLETED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }

	FpsSampler(int category, IEventListener* listener = NULL);
	virtual ~FpsSampler(void);
    
    static FpsSampler* createWithCategory(int category, IEventListener* listener = NULL, bool autorelease = true);

    void addSample(CCNode *sample);
    void clearSamples(void);
    
    void beginSampling(float duration, float delay = 0);
    double getRawFps(void) const;
    double getRecommendedFps(void) const;
    
    double getTargetFps(void) const { return mTargetFps; }
    void setTargetFps(double value);
    
	virtual void* getTarget(void) const { return mListener; }
	virtual void advanceTime(float dt);

	virtual bool isComplete(void) const { return !mIsSampling; }
	IEventListener* getListener(void) const { return mListener; }
	void setListener(IEventListener* value) { mListener = value; }

private:
    bool mIsSampling;
    int mNumFrameSkips;
    int mFrameCounter;
    float mDelay;
	float mDuration;
	float mTotalDuration;
    double mTargetFps;
    
	IEventListener* mListener;
};
#endif // __FPS_SAMPLER_H__
