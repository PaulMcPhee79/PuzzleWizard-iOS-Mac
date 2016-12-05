
#include "FloatTweener.h"
USING_NS_CC;

FloatTweener::FloatTweener(float startValue, IEventListener* listener, TRANS_FUNC transition)
{
    mOverflows = false;
	mInverted = false;
	mFrom = mTo = mValue = mPrevValue = startValue;
    mDuration = mTotalDuration = 0;
	mListener = listener;
	mTransition = transition ? transition : CMTransitions::LINEAR;
	mTag = -1;
    setAnimatableName("FloatTweener");
}

FloatTweener::~FloatTweener(void)
{
	mListener = NULL;
}

void FloatTweener::setValue(float value)
{
	mPrevValue = mValue;
	mValue = value;
}

void FloatTweener::setTransition(TRANS_FUNC value)
{
	mTransition = value ? value : CMTransitions::LINEAR;
}

void FloatTweener::reset(float value)
{
	mValue = mFrom = mTo = value;
	mDuration = mTotalDuration = 0;
}

void FloatTweener::syncWithTweener(FloatTweener* tweener)
{
    if (tweener != NULL && tweener != this)
    {
        this->setInverted(tweener->isInverted());
		reset(tweener->mFrom, tweener->mTo, tweener->mTotalDuration);
		mDuration = tweener->mDuration;
		updateValue();
    }
}

void FloatTweener::forceCompletion(void)
{
    if (!isComplete())
    {
        mDuration = mTotalDuration;
        updateValue();
    }
}

void FloatTweener::reset(float from, float to, float duration, float delay)
{
	mFrom = from;
    mTo = to;
    mDuration = -delay;
    mTotalDuration = MAX(0.01f, duration);

    if (mDuration >= 0)
	{
        mValue = mPrevValue = mFrom;
	}
}

void FloatTweener::updateValue(void)
{
	if (mDuration == mTotalDuration)
    {
        setValue(mTo);

        if (mListener)
		{
			mListener->onEvent(EV_TYPE_FLOAT_TWEENER_CHANGED(), this);
			mListener->onEvent(EV_TYPE_FLOAT_TWEENER_COMPLETED(), this);
		}
    }
    else if (mDuration >= 0 && mTotalDuration > 0)
    {
		float ratio = mDuration / mTotalDuration;
		float transitionValue = mInverted ? 1.0f - mTransition(1.0f - ratio) : mTransition(ratio);
        setValue(mFrom + (mTo - mFrom) * transitionValue);

		if (mListener)
        {
			mListener->onEvent(EV_TYPE_FLOAT_TWEENER_CHANGED(), this);
            
            if (mOverflows && mDuration >= mTotalDuration)
                mListener->onEvent(EV_TYPE_FLOAT_TWEENER_COMPLETED(), this);
        }
    }
}

void FloatTweener::advanceTime(float dt)
{
	if (!this->isComplete())
    {
        mDuration = mOverflows ? mDuration + dt : MIN(mTotalDuration, mDuration + dt);
		updateValue();
    }
}
