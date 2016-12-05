
#include "FpsSampler.h"
USING_NS_CC;

// This is commented out because I don't really care about devices that can't even run at 30fps
// and I'd prefer that a good device that lags badly during the sample is only restricted to
// 30fps at a minimum. Anything less than that on a good device and we will get rubbished for it,
// so it's not worth the risk to support a few extremely low performance devices.
//#define FPS_SAMPLER_MIN_20

static const int kDefaultFrameSkips = 10;
static const double kDefaultTargetFps = 60.0;

FpsSampler::FpsSampler(int category, IEventListener* listener)
:
Prop(category),
mListener(listener),
mIsSampling(false),
mNumFrameSkips(0),
mFrameCounter(0),
mDelay(0),
mDuration(0),
mTotalDuration(0),
mTargetFps(kDefaultTargetFps)
{
	setAnimatableName("FpsSampler");
}

FpsSampler::~FpsSampler(void)
{
	mListener = NULL;
}

FpsSampler* FpsSampler::createWithCategory(int category, IEventListener* listener, bool autorelease)
{
    FpsSampler *fpsSampler = new FpsSampler(category, listener);
	if (fpsSampler && fpsSampler->init())
    {
		if (autorelease)
			fpsSampler->autorelease();
        return fpsSampler;
    }
    CC_SAFE_DELETE(fpsSampler);
    return NULL;
}

void FpsSampler::setTargetFps(double value)
{
    if (value <= 0 || value == mTargetFps)
        return;
    
    // Only allow factors of 60
    double fpsFactor = kDefaultTargetFps / value;
    if (fabs(fpsFactor - (int)(fpsFactor + 0.01)) < 0.01)
        mTargetFps = (int)(value + 0.01);
    else
        CCLog("FpsSampler::setTargetFps - attempt to set invalid target FPS.");
}

void FpsSampler::addSample(CCNode *sample)
{
    if (sample)
        addChild(sample);
}

void FpsSampler::clearSamples(void)
{
    removeAllChildren();
}

void FpsSampler::beginSampling(float duration, float delay)
{
    CCAssert(mIsSampling == false, "FpsSampler::beginSampling - attempt to sample when sample is already in progress.");
    if (mIsSampling)
        return;
    
    mIsSampling = true;
    mNumFrameSkips = kDefaultFrameSkips;
    mFrameCounter = 0;
    mDuration = 0;
    mTotalDuration = duration;
    mDelay = delay;
}

double FpsSampler::getRawFps(void) const
{
    return mDuration > 0 ? (double)(mFrameCounter / mDuration) : (double)60;
}

double FpsSampler::getRecommendedFps(void) const
{
    double recFps = 60;
    double rawFps = getRawFps();
    double targetFps = getTargetFps();
    
    if (rawFps >= 0.825f * targetFps)
        recFps = targetFps;
#ifdef FPS_SAMPLER_MIN_20
    else if (rawFps >= 0.4f * targetFps)
            recFps = targetFps / 2;
        else
            recFps = targetFps / 3;
#else
    else
        	recFps = targetFps / 2;
#endif
    
    return recFps;
}

void FpsSampler::advanceTime(float dt)
{
    if (isComplete())
        return;
    
    if (mNumFrameSkips > 0)
    {
    	--mNumFrameSkips;
    	return;
    }

    // Delay is only rough, so it can delay for at most slightly
    // less than one frame longer than requested.
    if (mDelay > 0)
    {
        mDelay -= dt;
        return;
    }
    
    ++mFrameCounter;
    mDuration += dt;
    
    if (mDuration >= mTotalDuration)
    {
        mIsSampling = false;
        if (mListener)
            mListener->onEvent(EV_TYPE_FPS_SAMPLER_COMPLETED(), this);
    }
}
