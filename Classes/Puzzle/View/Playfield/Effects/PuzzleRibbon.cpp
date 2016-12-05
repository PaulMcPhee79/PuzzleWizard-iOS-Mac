
#include "PuzzleRibbon.h"
#include <Utils/ByteTweener.h>
#include <Utils/FloatTweener.h>
#include <Utils/Utils.h>
USING_NS_CC;

static const int kTweenerTagOpacity = 1;
static const int kTweenerTagLevel = 2;
static const int kTweenerTagPuzzle = 3;

static const float kBgQuadOpacityFactor = 0.7f;
static const float kPI = 3.14159265f;

PuzzleRibbon::PuzzleRibbon(int category)
	:
	Prop(category),
	mDuration(0.0f),
	mLevelLabel(NULL),
	mPuzzleLabel(NULL),
    mUpperStripe(NULL),
    mLowerStripe(NULL),
	mBgQuad(NULL),
	mOpacityTweener(NULL),
	mLevelLabelTweener(NULL),
	mPuzzleLabelTweener(NULL)
{
	mAdvanceable = true;
}

PuzzleRibbon::~PuzzleRibbon(void)
{
    mScene->deregisterLocalizable(this);
    
	if (mOpacityTweener)
		mOpacityTweener->setListener(NULL);
	CC_SAFE_RELEASE(mOpacityTweener);

	if (mLevelLabelTweener)
		mLevelLabelTweener->setListener(NULL);
	CC_SAFE_RELEASE(mLevelLabelTweener);

	if (mPuzzleLabelTweener)
		mPuzzleLabelTweener->setListener(NULL);
	CC_SAFE_RELEASE(mPuzzleLabelTweener);
}

bool PuzzleRibbon::init(void)
{
	if (mBgQuad)
		return true;

    const int kLabelReserveLen = 32;
    
	mBgQuad = CMUtils::createColoredQuad(CCSizeMake(mScene->getFullscreenWidth(), 184));
	mBgQuad->setPositionY(mScene->getViewHeight() / 2);
	mBgQuad->setColor(CMUtils::uint2color3B(0));
	mBgQuad->setOpacity((GLubyte)(kBgQuadOpacityFactor * 255));
	addChild(mBgQuad);

	mUpperStripe = CMUtils::createColoredQuad(CCSizeMake(mBgQuad->boundingBox().size.width, 10));
	mUpperStripe->setPositionY(mBgQuad->getPositionY() + mBgQuad->boundingBox().size.height / 2 - mUpperStripe->boundingBox().size.height / 2);
	mUpperStripe->setColor(mBgQuad->getColor());
	addChild(mUpperStripe);

	mLowerStripe = CMUtils::createColoredQuad(CCSizeMake(mBgQuad->boundingBox().size.width, 10));
	mLowerStripe->setPositionY(mBgQuad->getPositionY() - (mBgQuad->boundingBox().size.height / 2 + mLowerStripe->boundingBox().size.height / 2));
	mLowerStripe->setColor(mBgQuad->getColor());
	addChild(mLowerStripe);
    
    
    char labelStr[kLabelReserveLen+1] = ""; memset(labelStr, ' ', kLabelReserveLen * sizeof(char));
	mLevelLabel = TextUtils::createFX(labelStr, 70, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
    mLevelLabel->setString(" ");
	mLevelLabel->setPositionY(mBgQuad->getPositionY() + mBgQuad->boundingBox().size.height / 2 - (mLevelLabel->boundingBox().size.height / 2 + 16));
	addChild(mLevelLabel);

	mPuzzleLabel = TextUtils::create(labelStr, 54, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
    mPuzzleLabel->setString(" ");
	mPuzzleLabel->setPositionY(mLevelLabel->getPositionY() - 80);
	addChild(mPuzzleLabel);

	mOpacityTweener = new ByteTweener(0, this, CMTransitions::LINEAR);
	mOpacityTweener->setTag(kTweenerTagOpacity);

	mLevelLabelTweener = new FloatTweener(0, this, CMTransitions::LINEAR);
	mLevelLabelTweener->setTag(kTweenerTagLevel);

	mPuzzleLabelTweener = new FloatTweener(0, this, CMTransitions::LINEAR);
	mPuzzleLabelTweener->setTag(kTweenerTagPuzzle);

	setVisible(false);
    mScene->registerLocalizable(this);

	return true; // Obvious when it fails
}

void PuzzleRibbon::localeDidChange(const char* fontKey, const char* FXFontKey)
{
    TextUtils::switchFntFile(FXFontKey, mLevelLabel);
    TextUtils::switchFntFile(fontKey, mPuzzleLabel);
}

void PuzzleRibbon::setLevelString(const char* value)
{
	if (mLevelLabel)
		mLevelLabel->setString(SceneController::localizeString(value));
}

void PuzzleRibbon::setLevelColor(uint value)
{
	if (mLevelLabel)
		mLevelLabel->setColor(CMUtils::uint2color3B(value));
}

void PuzzleRibbon::setPuzzleString(const char* value)
{
	if (mPuzzleLabel)
		mPuzzleLabel->setString(mScene->localizeString(value));
}

void PuzzleRibbon::setPuzzleColor(uint value)
{
	if (mPuzzleLabel)
		mPuzzleLabel->setColor(CMUtils::uint2color3B(value));
}

void PuzzleRibbon::animate(float duration)
{
	if (mBgQuad == NULL || mOpacityTweener == NULL || mLevelLabelTweener == NULL || mPuzzleLabelTweener == NULL || mPuzzleLabel == NULL || mLevelLabel == NULL)
		return;

	stopAnimating();

	mDuration = duration;
	setOpacityChildren(0);
	mLevelLabel->setPositionX(mBgQuad->boundingBox().size.width / 2 + mLevelLabel->boundingBox().size.width / 2);
	mPuzzleLabel->setPositionX(-(mBgQuad->boundingBox().size.width / 2 + mPuzzleLabel->boundingBox().size.width / 2));

	mOpacityTweener->reset(getOpacity(), 255, 0.25f * duration);
	mLevelLabelTweener->reset(
		mLevelLabel->getPositionX(),
		-(mBgQuad->boundingBox().size.width / 2 + mLevelLabel->boundingBox().size.width / 2),
		duration);
	mPuzzleLabelTweener->reset(
		mPuzzleLabel->getPositionX(),
		mBgQuad->boundingBox().size.width / 2 + mPuzzleLabel->boundingBox().size.width / 2,
		duration);
	setVisible(true);
}

void PuzzleRibbon::stopAnimating(void)
{
	setVisible(false);
}

void PuzzleRibbon::resolutionDidChange(void)
{
    if (mBgQuad)
    {
        mBgQuad->setTextureRect(CCRectMake(0, 0, mScene->getFullscreenWidth(), mBgQuad->getTextureRect().size.height));
        mBgQuad->setPositionY(mScene->getViewHeight() / 2);
        
        if (mUpperStripe)
        {
            mUpperStripe->setTextureRect(CCRectMake(0, 0, mBgQuad->boundingBox().size.width, mUpperStripe->getTextureRect().size.height));
            mUpperStripe->setPositionY(mBgQuad->getPositionY() + mBgQuad->boundingBox().size.height / 2 - mUpperStripe->boundingBox().size.height / 2);
        }
        
        if (mLowerStripe)
        {
            mLowerStripe->setTextureRect(CCRectMake(0, 0, mBgQuad->boundingBox().size.width, mLowerStripe->getTextureRect().size.height));
            mLowerStripe->setPositionY(mBgQuad->getPositionY() - (mBgQuad->boundingBox().size.height / 2 + mLowerStripe->boundingBox().size.height / 2));
        }
    }
}

void PuzzleRibbon::onEvent(int evType, void* evData)
{
	if (evType == ByteTweener::EV_TYPE_BYTE_TWEENER_CHANGED())
	{
		float tweenedValue = mOpacityTweener->getTweenedValue();
		setOpacityChildren(tweenedValue);
		mBgQuad->setOpacity((GLubyte)(kBgQuadOpacityFactor * tweenedValue));
	}
	else if (evType == ByteTweener::EV_TYPE_BYTE_TWEENER_COMPLETED())
	{
		if (getOpacity() < 128)
			setVisible(false);
		else
			mOpacityTweener->reset(getOpacity(), 0, 0.25f * mDuration, 0.5f * mDuration);
	}
	else if (evType == FloatTweener::EV_TYPE_FLOAT_TWEENER_CHANGED())
	{
		FloatTweener* tweener = static_cast<FloatTweener*>(evData);
		if (tweener)
		{
			float tweenedValue = tweener->getTweenedValue();
			switch (tweener->getTag())
			{
				case kTweenerTagLevel:
					mLevelLabel->setPositionX(tweenedValue);
					break;
				case kTweenerTagPuzzle:
					mPuzzleLabel->setPositionX(tweenedValue);
					break;
			}
		}
	}
}

void PuzzleRibbon::advanceTime(float dt)
{
	if (!isVisible() || mLevelLabelTweener == NULL)
		return;
	
	float adjustedTime = 1.5f * MAX(0.025f * dt, dt * fabs(cosf(mPuzzleLabelTweener->getPercentComplete() * kPI)));
	if (mOpacityTweener)
		mOpacityTweener->advanceTime(adjustedTime);
	if (mLevelLabelTweener)
		mLevelLabelTweener->advanceTime(adjustedTime);
	if (mPuzzleLabelTweener)
		mPuzzleLabelTweener->advanceTime(adjustedTime);
}
