#ifndef __PARTICLE_PROP_H__
#define __PARTICLE_PROP_H__

#include "cocos2d.h"
#include "Prop.h"
#include <Events/EventDispatcher.h>

USING_NS_CC;

class ParticleProp : public Prop
{
public:
    static int EV_TYPE_DID_COMPLETE() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
    
	ParticleProp(int category = -1, IEventListener* listener = NULL);
	virtual ~ParticleProp(void);

	static ParticleProp* createWithCategory(int category, const char *plistFile, IEventListener* listener, bool autorelease = true);
	virtual bool initWithPlistFile(const char *plistFile);

    void setListener(IEventListener* listener);
    
    bool isActive(void) const { return mIsActive; }
    float getDurationRemaining(void) { return MAX(0, mTotalDuration - mElapsedDuration); }
    float getTotalDuration(void) { return mTotalDuration; }
    virtual void* getTarget(void) const { return mListener; }
    CCParticleSystem* getSystem(void) const { return mSystem; }
    
    void stopSystem(void);
    void resetSystem(float delay = 0.0f);
    
	virtual void advanceTime(float dt);

protected:
    void setElapsedDuration(float value);
    
private:
    bool mIsActive;
    float mDelay;
    float mElapsedDuration;
    float mTotalDuration;
    CCParticleSystemQuad* mSystem;
    CCParticleBatchNode* mBatch;
	IEventListener* mListener;
};
#endif // __PARTICLE_PROP_H__
