
#include "SolvedAnimation.h"
#include <Puzzle/View/Playfield/Effects/Twinkle.h>
#include <Utils/ByteTweener.h>
#include <Utils/FloatTweener.h>
USING_NS_CC;

static const int kAnimIndexX = 0;
static const int kAnimIndexY = 1;
static const int kAnimIndexScaleX = 2;
static const int kAnimIndexRotation = 3;

static const float kTwinkleDuration = 1.0f;

#pragma warning( disable : 4351 ) // new behavior: elements of array 'xyz' will be default initialized (changed in Visual C++ 2005) 
SolvedAnimation::SolvedAnimation(int category, IEventListener* listener)
	:
	Prop(category),
	mListener(listener),
	mIsAnimating(false),
	mTwinkle(NULL),
    mTwinkleScaler(NULL),
	mKey(NULL),
	mFadeTweener(NULL),
	mAnimTweeners()
{

}

SolvedAnimation::~SolvedAnimation(void)
{
	stopAnimating();

	if (mTwinkle)
	{
		mTwinkle->setListener(NULL);
        mTwinkle->returnToPool();
        CC_SAFE_RELEASE_NULL(mTwinkle);
	}
    
    CC_SAFE_RELEASE_NULL(mTwinkleScaler);
	
	for (int i = 0; i < kNumAnimTweeners; ++i)
		CC_SAFE_RELEASE_NULL(mAnimTweeners[i]);

	CC_SAFE_RELEASE_NULL(mFadeTweener);
	mListener = NULL;
}

SolvedAnimation* SolvedAnimation::create(int category, IEventListener* listener, bool autorelease)
{
	SolvedAnimation *solvedAnim = new SolvedAnimation(category, listener);
	if (solvedAnim && solvedAnim->init())
    {
		if (autorelease)
			solvedAnim->autorelease();
        return solvedAnim;
    }
    CC_SAFE_DELETE(solvedAnim);
    return NULL;
}

bool SolvedAnimation::init(void)
{
	if (mKey)
		return true;

	mKey = CCSprite::createWithSpriteFrame(mScene->spriteFrameByName("solved-key"));
	addChild(mKey);

	mTwinkle = Twinkle::getTwinkle(CCPointZero);
	mTwinkle->retain();
	mTwinkle->setListener(this);
	//addChild(mTwinkle);
    
    mTwinkleScaler = new CCNode();
    mTwinkleScaler->setPosition(ccp(mKey->boundingBox().size.width / 3, 10));
    mTwinkleScaler->setScale(1.5f);
    mTwinkleScaler->addChild(mTwinkle);
    addChild(mTwinkleScaler);

	mFadeTweener = new ByteTweener(0, this, CMTransitions::LINEAR);

	for (int i = 0; i < kNumAnimTweeners; ++i)
	{
		mAnimTweeners[i] = new FloatTweener(0, this, CMTransitions::LINEAR);
		mAnimTweeners[i]->setTag(i);
	}

	setRotation(-35);
	setScale(64.0f / mKey->boundingBox().size.width);

	return true; // Obvious when it fails
}

void SolvedAnimation::animate(const CCPoint& from, const CCPoint& to, float duration, float delay)
{
	if (isAnimating() || mKey == NULL || getParent() == NULL)
		return;

	CCPoint localFrom = getParent()->convertToNodeSpace(from);
	setRotation(-35);
	setScale(64.0f / mKey->boundingBox().size.width);
	setPosition(localFrom);
	setOpacityChildren(255);

	for (int i = 0; i < kNumAnimTweeners; ++i)
	{
		mScene->removeFromJuggler(mAnimTweeners[i]);

		switch (i)
		{
			case kAnimIndexX:
				mAnimTweeners[i]->reset(localFrom.x, to.x, duration, delay);
				break;
			case kAnimIndexY:
				mAnimTweeners[i]->reset(localFrom.y, to.y, duration, delay);
				break;
			case kAnimIndexScaleX:
				mAnimTweeners[i]->reset(getScale(), 1, duration, delay);
				break;
			case kAnimIndexRotation:
				mAnimTweeners[i]->reset(getRotation(), -295, duration, delay);
				break;
		}

		mScene->addToJuggler(mAnimTweeners[i]);
	}

	setAnimating(true);
}

void SolvedAnimation::stopAnimating(void)
{
	for (int i = 0; i < kNumAnimTweeners; ++i)
		mScene->removeFromJuggler(mAnimTweeners[i]);

	mScene->removeFromJuggler(mFadeTweener);
	setAnimating(false);
}

void SolvedAnimation::fadeOverTime(float duration, float delay)
{
	mScene->removeFromJuggler(mFadeTweener);
	mFadeTweener->reset(getOpacity(), 0, duration, delay);
	mScene->addToJuggler(mFadeTweener);
}

void SolvedAnimation::onEvent(int evType, void* evData)
{
	if (evType == FloatTweener::EV_TYPE_FLOAT_TWEENER_CHANGED())
	{
		FloatTweener* tweener = static_cast<FloatTweener*>(evData);
		if (tweener)
		{
			float tweenedValue = tweener->getTweenedValue();
			switch (tweener->getTag())
			{
				case kAnimIndexX:
					setPositionX(tweenedValue);
					break;
				case kAnimIndexY:
					setPositionY(tweenedValue);
					break;
				case kAnimIndexScaleX:
					setScale(tweenedValue);
					break;
				case kAnimIndexRotation:
					setRotation(tweenedValue);
					break;
			}
		}
	}
	else if (evType == FloatTweener::EV_TYPE_FLOAT_TWEENER_COMPLETED())
	{
		FloatTweener* tweener = static_cast<FloatTweener*>(evData);
		if (tweener && tweener->getTag() == kAnimIndexRotation)
		{
			if (mTwinkle)
				mTwinkle->animate(kTwinkleDuration);
		}
	}
	else if (evType == Twinkle::EV_TYPE_ANIMATION_COMPLETED())
	{
		fadeOverTime(0.5f, 1.5f);
	}
	else if (evType == ByteTweener::EV_TYPE_BYTE_TWEENER_CHANGED())
	{
		setOpacityChildren(mFadeTweener->getTweenedValue());
	}
	else if (evType == ByteTweener::EV_TYPE_BYTE_TWEENER_COMPLETED())
	{
        if (isAnimating())
        {
            setAnimating(false);
            if (mListener)
                mListener->onEvent(EV_TYPE_ANIMATION_COMPLETED(), this);
        }
	}
}
