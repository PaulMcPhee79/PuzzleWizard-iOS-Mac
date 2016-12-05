
#include "Twinkle.h"
#include <Utils/ReusableCache.h>
#include <Utils/FloatTweener.h>
#include <Utils/ByteTweener.h>
USING_NS_CC;

static const int kRotationIndex = 0;
static const int kOpacityIndex = 1;
static const int kScaleIndex = 2;
static const int kNumTweenerCounters = 3;

#pragma warning( disable : 4351 ) // new behavior: elements of array 'Twinkle::mTweenerCounters' will be default initialized (changed in Visual C++ 2005) 
Twinkle::Twinkle(const CCPoint& origin)
	:
mIsAnimating(false),
mDuration(0),
mDurationPassed(0),
mTweenerCounters(),
mListener(NULL),
mRotateTweener(NULL),
mScaleTweener(NULL),
mOpacityTweener(NULL)
{
	setPosition(origin);
    setAnimatableName("Twinkle");
}

Twinkle::~Twinkle(void)
{
    mIsAnimating = false;
    removeFromParent();
	mScene->removeFromJuggler(this);
	mListener = NULL;
	CC_SAFE_RELEASE_NULL(mRotateTweener);
	CC_SAFE_RELEASE_NULL(mScaleTweener);
	CC_SAFE_RELEASE_NULL(mOpacityTweener);
}

Twinkle* Twinkle::create(const CCPoint& origin, bool autorelease)
{
	Twinkle *twinkle = new Twinkle(origin);
    if (twinkle && twinkle->init())
    {
		if (autorelease)
			twinkle->autorelease();
        return twinkle;
    }
    CC_SAFE_DELETE(twinkle);
    return NULL;
}

bool Twinkle::init(void)
{
	bool bRet = CMSprite::init();
	if (bRet)
	{
		setDisplayFrame(mScene->spriteFrameByName("twinkle"));
		mRotateTweener = new FloatTweener(0, this);
		mScaleTweener = new FloatTweener(0, this, CMTransitions::EASE_OUT);
		mOpacityTweener = new ByteTweener(0, this, CMTransitions::EASE_OUT);
        
        setScale(0);
        setRotation(0);
        setOpacity(0);
	}

	return bRet;
}

void Twinkle::setAnimating(bool value)
{
    mIsAnimating = value;
    if (!value)
    {
        setScale(0.0f);
        setRotation(0.0f);
        setOpacity(0);
    }
}

void Twinkle::animate(float duration)
{
	if (isAnimating())
		mScene->removeFromJuggler(this);

	setAnimating(true);
	mDuration = duration;
	mDurationPassed = 0.0f;
	setScale(0);
	setRotation(0);
	setOpacity(0);

	for (int i = 0; i < kNumTweenerCounters; ++i)
		mTweenerCounters[i] = 0;
	
	mRotateTweener->reset(getRotation(), 360, 0.75f * duration);

	mScaleTweener->setInverted(false);
	mScaleTweener->reset(getScale(), 1.25f, 0.5f * duration);

	mOpacityTweener->setInverted(false);
	mOpacityTweener->reset(getOpacity(), 255, 0.5f * duration);

	mScene->addToJuggler(this);
}

void Twinkle::fastForward(float duration)
{
	if (!isAnimating())
		return;

	float adjustedDurationPassed = mDurationPassed + duration;
	mDurationPassed = MIN(adjustedDurationPassed, mDuration);
	if (adjustedDurationPassed >= mDuration)
    {
        stopAnimating();
        return;
    }

	// Rotation
    if (adjustedDurationPassed < 0.75f * mDuration)
    {
        mRotateTweener->advanceTime(duration);
        setRotation(mRotateTweener->getTweenedValue());
    }
    else
    {
        setRotation(0.0f);
        mTweenerCounters[kRotationIndex] = 1;
        mRotateTweener->reset(getRotation(), 360, 0.75f * mDuration);
        mRotateTweener->advanceTime(adjustedDurationPassed - 0.75f * mDuration);
		setRotation(mRotateTweener->getTweenedValue());
    }

    // Opacity + Scale
    if (adjustedDurationPassed < 0.5f * mDuration)
    {
        mOpacityTweener->advanceTime(duration);
        setOpacity(mOpacityTweener->getTweenedValue());

        mScaleTweener->advanceTime(duration);
        setScale(mScaleTweener->getTweenedValue());
    }
    else
    {
        setOpacity(255);
        setScale(1.25f);
        mTweenerCounters[kOpacityIndex] = mTweenerCounters[kScaleIndex] = 1;

        mOpacityTweener->setInverted(true);
        mOpacityTweener->reset(getOpacity(), 0, 0.5f * mDuration);
        mOpacityTweener->advanceTime(adjustedDurationPassed - 0.5f * mDuration);
        setOpacity(mOpacityTweener->getTweenedValue());

        mScaleTweener->setInverted(true);
        mScaleTweener->reset(getScale(), 0.0f, 0.5f * mDuration);
        mScaleTweener->advanceTime(adjustedDurationPassed - 0.5f * mDuration);
        setScale(mScaleTweener->getTweenedValue());
    }
}

void Twinkle::stopAnimating(void)
{
	if (isAnimating())
	{
		mScene->removeFromJuggler(this);

		setAnimating(false);
		if (mListener)
			mListener->onEvent(EV_TYPE_ANIMATION_COMPLETED(), this);
	}
}

void Twinkle::syncWithTwinkle(Twinkle* twinkle)
{
	if (twinkle == NULL)
        return;

    if (isAnimating() && !twinkle->isAnimating())
    {
        mScene->removeFromJuggler(this);
        setAnimating(false);
    }
    else if (!isAnimating() && twinkle->isAnimating())
    {
        mDuration = twinkle->mDuration;
        mDurationPassed = twinkle->mDurationPassed;
        setScale(twinkle->getScale());
        setRotation(twinkle->getRotation());
        setOpacity(twinkle->getOpacity());
        
        for (int i = 0; i < kNumTweenerCounters; ++i)
            mTweenerCounters[i] = twinkle->mTweenerCounters[i];
        
        mOpacityTweener->syncWithTweener(twinkle->mOpacityTweener);
        mRotateTweener->syncWithTweener(twinkle->mRotateTweener);
        mScaleTweener->syncWithTweener(twinkle->mScaleTweener);
        
        setAnimating(true);
        mScene->addToJuggler(this);
    }
}

void Twinkle::onEvent(int evType, void* evData)
{
	if (evType == FloatTweener::EV_TYPE_FLOAT_TWEENER_CHANGED())
	{
		FloatTweener* tweener = static_cast<FloatTweener*>(evData);
		if (tweener)
		{
			if (tweener == mRotateTweener)
				setRotation(tweener->getTweenedValue());
			else if (tweener == mScaleTweener)
				setScale(tweener->getTweenedValue());
		}
	}
	else if (evType == ByteTweener::EV_TYPE_BYTE_TWEENER_CHANGED())
	{
		ByteTweener* tweener = static_cast<ByteTweener*>(evData);
		if (tweener && tweener == mOpacityTweener)
			setOpacity(tweener->getTweenedValue());
	}
	else if (evType == FloatTweener::EV_TYPE_FLOAT_TWEENER_COMPLETED())
	{
		FloatTweener* tweener = static_cast<FloatTweener*>(evData);
		if (tweener)
		{
			if (tweener == mRotateTweener)
			{
				++mTweenerCounters[kRotationIndex];
				if (mTweenerCounters[kRotationIndex] == 1)
					mRotateTweener->reset(getRotation(), 720, 0.75f * mDuration);
			}
			else if (tweener == mScaleTweener)
			{
				++mTweenerCounters[kScaleIndex];
				if (mTweenerCounters[kScaleIndex] == 1)
				{
					mScaleTweener->setInverted(true);
					mScaleTweener->reset(getScale(), 0.0f, 0.5f * mDuration);
				}
				else
					stopAnimating();
			}
		}
	}
	else if (evType == ByteTweener::EV_TYPE_BYTE_TWEENER_COMPLETED())
	{
		ByteTweener* tweener = static_cast<ByteTweener*>(evData);
		if (tweener && tweener == mOpacityTweener)
		{
			++mTweenerCounters[kOpacityIndex];
			if (mTweenerCounters[kOpacityIndex] == 1)
			{
				mOpacityTweener->setInverted(true);
				mOpacityTweener->reset(getOpacity(), 0, 0.5f * mDuration);
			}
		}
	}
}

void Twinkle::advanceTime(float dt)
{
	if (isAnimating())
	{
		mDurationPassed += dt;
        //if (mOpacityTweener)
            mOpacityTweener->advanceTime(dt);
        //if (mRotateTweener)
            mRotateTweener->advanceTime(dt);
        //if (mScaleTweener)
            mScaleTweener->advanceTime(dt);
	}
}

/* Reusable Implementation */
ReusableCache* Twinkle::sCache = NULL;
bool Twinkle::sCaching = false;

void Twinkle::setupReusables(void)
{
	if (sCache)
        return;

    sCaching = true;
    sCache = new ReusableCache();

#if CM_SMALL_CACHES
    int cacheSize = 10;
#else
    int cacheSize = 15;
#endif
    uint reuseKey = 0;
    IReusable* reusable = NULL;
    sCache->addKey(cacheSize, reuseKey);
	CCPoint origin = ccp(0, 0);

    for (int i = 0; i < cacheSize; ++i)
    {
        reusable = getTwinkle(origin);
        reusable->hibernate();
        sCache->addReusable(reusable);
    }

    sCache->verifyCacheIntegrity();
    sCaching = false;
}

IReusable* Twinkle::checkoutReusable(uint reuseKey)
{
	IReusable* reusable = NULL;

    if (sCache && !sCaching)
		reusable = sCache->checkout(reuseKey);

    return reusable;
}

void Twinkle::checkinReusable(IReusable* reusable)
{
	if (sCache && !sCaching && reusable)
		sCache->checkin(reusable);
}

Twinkle* Twinkle::getTwinkle(const CCPoint& origin)
{
	uint reuseKey = 0;
    Twinkle* twinkle = static_cast<Twinkle*>(checkoutReusable(reuseKey));

    if (twinkle)
    {
        twinkle->reuse();
        twinkle->setPosition(origin);
    }
    else
    {
        twinkle = Twinkle::create(origin, !sCaching);
    }

    return twinkle;
}

void Twinkle::reuse(void)
{
	if (getInUse())
         return;

     setScale(0);
     setRotation(0);
     setOpacity(0);
     setVisible(true);
     mInUse = true;
}

void Twinkle::hibernate(void)
{
	if (!getInUse())
        return;

	removeFromParent();
	mScene->removeFromJuggler(this);
	setAnimating(false);
    mListener = NULL;

    mInUse = false;
    checkinReusable(this);
}

void Twinkle::returnToPool(void)
{
    if (getPoolIndex() != -1)
        hibernate();
    else
        removeFromParent();
}
