
#include "ParticleProp.h"
#include <Utils/Utils.h>
USING_NS_CC;

ParticleProp::ParticleProp(int category, IEventListener* listener)
    : Prop(category)
    , mIsActive(true)
    , mDelay(0)
    , mElapsedDuration(0)
    , mTotalDuration(0)
    , mSystem(NULL)
    , mBatch(NULL)
    , mListener(listener)
{
    mAdvanceable = true;
}

ParticleProp::~ParticleProp(void)
{
	mListener = NULL;
}

ParticleProp* ParticleProp::createWithCategory(int category, const char *plistFile, IEventListener* listener, bool autorelease)
{
    ParticleProp *prop = new ParticleProp(category, listener);
	if (prop && prop->initWithPlistFile(plistFile))
    {
		if (autorelease)
			prop->autorelease();
        return prop;
    }
    CC_SAFE_DELETE(prop);
    return NULL;
}

bool ParticleProp::initWithPlistFile(const char *plistFile)
{
    if (!plistFile)
        return false;
    
    bool bRet = true;
    
    do
    {
        mSystem = new CCParticleSystemQuad();
        bRet = mSystem && mSystem->initWithFile(plistFile);
        if (!bRet) break;
        mSystem->autorelease();
        mTotalDuration = mSystem->getLife() + mSystem->getLifeVar();
        
        mBatch = CCParticleBatchNode::createWithTexture(mSystem->getTexture());
        if (!mBatch) { bRet = false; break; }
        mBatch->addChild(mSystem);
        addChild(mBatch);
    }
    while (false);
    
    return bRet;
}

void ParticleProp::setElapsedDuration(float value)
{
    if (value != mElapsedDuration)
    {
        float prevElapsedDuration = mElapsedDuration;
        mElapsedDuration = value;
        if (isActive() && prevElapsedDuration < mTotalDuration && mElapsedDuration >= mTotalDuration)
        {
            stopSystem();
            if (mListener)
                mListener->onEvent(EV_TYPE_DID_COMPLETE(), (void*)this);
        }
    }
}

void ParticleProp::stopSystem(void)
{
    mIsActive = false;
    mSystem->stopSystem();
}

void ParticleProp::resetSystem(float delay)
{
    mIsActive = true;
    mDelay = delay;
    mElapsedDuration = 0; // Don't use setter here
    if (mDelay == 0)
        mSystem->resetSystem();
}

void ParticleProp::advanceTime(float dt)
{
    if (isActive())
    {
        if (mDelay > 0)
        {
            mDelay -= dt;
            if (mDelay <= 0)
            {
                dt = -mDelay;
                mSystem->resetSystem();
            }
        }
        
        if (mDelay <= 0)
            setElapsedDuration(mElapsedDuration + dt);
    }
}
