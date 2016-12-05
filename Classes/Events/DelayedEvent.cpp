
#include "DelayedEvent.h"
#include <Interfaces/IEventListener.h>
USING_NS_CC;


DelayedEvent::DelayedEvent(int eventTag, float delay, IEventListener* listener, void* userInfo)
	: Prop(-1)
    , mEventTag(eventTag)
    , mDelay(delay)
    , mListener(listener)
    , mUserInfo(userInfo)
{
    
}

DelayedEvent* DelayedEvent::create(int eventTag, float delay, IEventListener* listener, void* userInfo)
{
    DelayedEvent *ev = new DelayedEvent(eventTag, delay, listener, userInfo);
	if (ev && ev->init())
        return ev;
    CC_SAFE_DELETE(ev);
    return NULL;
}

DelayedEvent::~DelayedEvent(void)
{
    mScene->removeFromJuggler(this);
	mListener = NULL;
    mUserInfo = NULL;
}

void DelayedEvent::advanceTime(float dt)
{
    if (!isComplete())
    {
        mDelay -= dt;
        if (isComplete())
        {
            if (mListener)
                mListener->onEvent(EV_TYPE_DID_EXPIRE(), (void*)this);
            autorelease();
        }
    }
}
