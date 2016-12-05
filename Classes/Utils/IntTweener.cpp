
#include "IntTweener.h"
USING_NS_CC;

IntTweener::IntTweener(int startValue, IEventListener* listener, TRANS_FUNC transition)
{
	mInverted = false;
	mFrom = mTo = mValue = startValue;
    mDuration = mTotalDuration = 0;
	mListener = listener;
	mTransition = transition;
	mTag = -1;
    setAnimatableName("IntTweener");
}

IntTweener::~IntTweener(void)
{
	mListener = NULL;
}

void IntTweener::reset(int value)
{
	mValue = mFrom = mTo = value;
	mDuration = mTotalDuration = 0;
}

void IntTweener::reset(int from, int to, float duration, float delay)
{
	mFrom = from;
    mTo = to;
    mDuration = -delay;
    mTotalDuration = MAX(0.01f, duration);

    if (mDuration >= 0)
        mValue = mFrom;
}

void IntTweener::advanceTime(float dt)
{
	if (!this->isComplete())
    {
        mDuration = MIN(mTotalDuration, mDuration + dt);

        if (mDuration == mTotalDuration)
        {
            mValue = mTo;

            if (mListener)
			{
				mListener->onEvent(EV_TYPE_INT_TWEENER_CHANGED(), this);
				mListener->onEvent(EV_TYPE_INT_TWEENER_COMPLETED(), this);
			}
        }
        else if (mDuration >= 0 && mTotalDuration > 0)
        {
			int prevValue = mValue;
            float ratio = mDuration / mTotalDuration;
			float transitionValue = mInverted ? 1.0f - mTransition(1.0f - ratio) : mTransition(ratio);
            mValue = (int)(mFrom + (mTo - mFrom) * transitionValue);

			if (mListener && mValue != prevValue)
				mListener->onEvent(EV_TYPE_INT_TWEENER_CHANGED(), this);
        }
    }
}
