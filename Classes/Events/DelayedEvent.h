#ifndef __DELAYED_EVENT_H__
#define __DELAYED_EVENT_H__

#include "cocos2d.h"
#include <Prop/Prop.h>
#include "EventDispatcher.h"
USING_NS_CC;

class IEventListener;

// NOTE: *****
// Set and (mostly) forget. Not intended to be managed.
// Workflow: Create->Add to Juggler->[Receive Event]->Cleans self up->Client Juggler::removeTweensWithTarget(static_cast<IEventListener*>())
class DelayedEvent : public Prop // Hack: Prop has what we need.
{
public:
    static int EV_TYPE_DID_EXPIRE() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
    
    static DelayedEvent* create(int eventTag, float delay, IEventListener* listener, void* userInfo = NULL);
	virtual ~DelayedEvent(void);
    
    int getEventTag(void) const { return mEventTag; }
    void* getUserInfo(void) const { return mUserInfo; }
    
    virtual bool isComplete(void) const { return mDelay <= 0; }
    virtual void* getTarget(void) const { return mListener; }
    virtual void advanceTime(float dt);
    
protected:
    DelayedEvent(int eventTag, float delay, IEventListener* listener, void* userInfo = NULL);
    
private:
    int mEventTag;
    float mDelay;
    void* mUserInfo;
	IEventListener* mListener;
};
#endif // __DELAYED_EVENT_H__
