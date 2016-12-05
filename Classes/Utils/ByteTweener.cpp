
#include "ByteTweener.h"
USING_NS_CC;

ByteTweener::ByteTweener(GLubyte startValue, IEventListener* listener, TRANS_FUNC transition)
{
	mInverted = false;
	mFrom = mTo = mValue = startValue;
    mDuration = mTotalDuration = 0;
	mListener = listener;
	mTransition = transition;
	mTag = -1;
    setAnimatableName("ByteTweener");
}

ByteTweener::~ByteTweener(void)
{
	mListener = NULL;
}

void ByteTweener::reset(GLubyte value)
{
	mValue = mFrom = mTo = value;
	mDuration = mTotalDuration = 0;
}

void ByteTweener::reset(GLubyte from, GLubyte to, float duration, float delay)
{
	mFrom = from;
    mTo = to;
    mDuration = -delay;
    mTotalDuration = MAX(0.01f, duration);

    if (mDuration >= 0)
        mValue = mFrom;
}

void ByteTweener::reverse(void)
{
	reset(mTo, mFrom, mTotalDuration);
}

void ByteTweener::syncWithTweener(ByteTweener* tweener)
{
	if (tweener && tweener != this)
	{
		this->setInverted(tweener->isInverted());
		reset(tweener->mFrom, tweener->mTo, tweener->mTotalDuration);
		mDuration = tweener->mDuration;
		updateValue();
	}
}

void ByteTweener::updateValue(void)
{
	if (mDuration == mTotalDuration)
    {
        mValue = mTo;

        if (mListener)
		{
			mListener->onEvent(EV_TYPE_BYTE_TWEENER_CHANGED(), this);
			mListener->onEvent(EV_TYPE_BYTE_TWEENER_COMPLETED(), this);
		}
    }
    else if (mDuration >= 0 && mTotalDuration > 0)
    {
		GLubyte prevValue = mValue;
        float ratio = mDuration / mTotalDuration;
		float transitionValue = mInverted ? 1.0f - mTransition(1.0f - ratio) : mTransition(ratio);
        mValue = (GLubyte)MIN(255, mFrom + (mTo - mFrom) * transitionValue);

		if (mListener && mValue != prevValue)
			mListener->onEvent(EV_TYPE_BYTE_TWEENER_CHANGED(), this);
    }
}

void ByteTweener::advanceTime(float dt)
{
	if (!this->isComplete())
    {
        mDuration = MIN(mTotalDuration, mDuration + dt);
		updateValue();
    }
}
