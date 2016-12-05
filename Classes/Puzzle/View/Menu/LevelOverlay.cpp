
#include "LevelOverlay.h"
#include <Utils/ByteTweener.h>
#include <Utils/Utils.h>
#include <Utils/CMTypes.h>

static const int kNumRunes = 4;
static const int kRuneSequenceLength = 120;
static const float kRuneTweenDuration = 1.5f;
static const int kRuneSequence[kRuneSequenceLength] =
{
    0, 1, 2, 3, -1,
    2, 1, 3, 0, -1,
    3, 2, 1, 0, -1,
    1, 2, 3, 0, -1,
    3, 1, 0, 2, -1,
    0, 3, 2, 1, -1,
    
    1, 0, 2, 3, -1,
    2, 0, 3, 1, -1,
    1, 2, 0, 3, -1,
    0, 2, 3, 1, -1,
    3, 0, 1, 2, -1,
    1, 3, 2, 0, -1,
    
    2, 1, 0, 3, -1,
    0, 1, 3, 2, -1,
    2, 0, 1, 3, -1,
    1, 0, 3, 2, -1,
    0, 2, 1, 3, -1,
    3, 0, 2, 1, -1,
    
    3, 1, 2, 0, -1,
    0, 3, 1, 2, -1,
    2, 3, 1, 0, -1,
    3, 2, 0, 1, -1,
    1, 3, 0, 2, -1,
    2, 3, 0, 1, -1
};

LevelOverlay::LevelOverlay(int category)
: Prop(category)
, mRuneIndex(0)
, mState(LevelOverlay::IDLE)
, mWizardHat(NULL)
, mRuneTweener(NULL)
{
    mRuneIndex = CMUtils::nextRandom(0, kRuneSequenceLength - 1);
    mRuneIndex = MIN(kRuneSequenceLength - 1, MAX(0, mRuneIndex - mRuneIndex % (kNumRunes + 1)));
}

LevelOverlay::~LevelOverlay(void)
{
    if (mRuneTweener)
    {
        mRuneTweener->setListener(NULL);
        CC_SAFE_RELEASE_NULL(mRuneTweener);
    }
    
    for (std::vector<CCSprite*>::iterator it = mRunes.begin(); it != mRunes.end(); ++it)
        CC_SAFE_RELEASE(*it);
    mRunes.clear();
    
    CC_SAFE_RELEASE_NULL(mWizardHat);
}

bool LevelOverlay::init(void)
{
    if (mWizardHat)
        return true;
    
    mWizardHat = CCSprite::createWithSpriteFrame(mScene->spriteFrameByName("level-hat"));
    mWizardHat->retain();
    addChild(mWizardHat);
    
    vec2 runePositions[kNumRunes] = { cmv2(54, 90), cmv2(68.5f, 97.5f), cmv2(89.5f, 106.5f), cmv2(112.5f, 106.5f) };
    for (int i = 0; i < kNumRunes; ++i)
    {
        CCSprite* rune = CCSprite::createWithSpriteFrame(mScene->spriteFrameByName(CMUtils::strConcatVal("hat-glow-", i).c_str()));
        rune->retain();
        rune->setPosition(ccp(
                              mWizardHat->getPositionX() + rune->boundingBox().size.width / 2 + runePositions[i].x - mWizardHat->boundingBox().size.width / 2,
                              mWizardHat->getPositionY() + mWizardHat->boundingBox().size.height / 2 - (rune->boundingBox().size.height / 2 + runePositions[i].y)));
        rune->setOpacity(0);
        mRunes.push_back(rune);
        addChild(rune);
    }
    
    mRuneTweener = new ByteTweener(0, this, CMTransitions::LINEAR);
    
    setState(LevelOverlay::GLOW);
    return true; // We'll know immediately if it fails anyway.
}

void LevelOverlay::setState(LevelOverlayState value)
{
    if (mState == value)
        return;
    
    if (mRuneTweener && (mState == LevelOverlay::FADE || mState == LevelOverlay::GLOW))
    {
        for (int i = 0; i < mRunes.size(); ++i)
        {
            if (i == kRuneSequence[mRuneIndex] || kRuneSequence[mRuneIndex] == -1)
                mRunes[i]->setOpacity(mRuneTweener->getTweenedValue());
        }
    }
    
    switch (value)
    {
        case LevelOverlay::IDLE:
            for (std::vector<CCSprite*>::iterator it = mRunes.begin(); it != mRunes.end(); ++it)
                (*it)->setOpacity(0);
            break;
        case LevelOverlay::GLOW:
            if (++mRuneIndex >= kRuneSequenceLength)
                mRuneIndex = 0;
            
            if (mRuneTweener)
            {
                if (mState == LevelOverlay::FADE)
                    mRuneTweener->reverse();
                else
                    mRuneTweener->reset(0, 255, kRuneTweenDuration);
            }
            break;
        case LevelOverlay::FADE:
            if (mRuneTweener)
            {
                if (mState == LevelOverlay::GLOW)
                    mRuneTweener->reverse();
                else
                    mRuneTweener->reset(255, 0, kRuneTweenDuration);
            }
            break;
    }
    
    mState = value;
}

void LevelOverlay::setOpacity(GLubyte value)
{
    Prop::setOpacity(value);
    
    if (mWizardHat)
        mWizardHat->setOpacity(value);
    
    for (std::vector<CCSprite*>::iterator it = mRunes.begin(); it != mRunes.end(); ++it)
        (*it)->setOpacity((GLubyte)((*it)->getOpacity() * (value / 255.0f)));
}

CCRect LevelOverlay::getContentBounds(void)
{
    if (mWizardHat)
        return mWizardHat->boundingBox();
    else
        return CCRectZero;
}

void LevelOverlay::onEvent(int evType, void* evData)
{
    if (evType == ByteTweener::EV_TYPE_BYTE_TWEENER_CHANGED())
	{
		ByteTweener* tweener = static_cast<ByteTweener*>(evData);
		if (tweener)
		{
            for (int i = 0; i < mRunes.size(); ++i)
            {
                if (i == kRuneSequence[mRuneIndex] || kRuneSequence[mRuneIndex] == -1)
                    mRunes[i]->setOpacity(tweener->getTweenedValue());
            }
        }
	}
	else if (evType == ByteTweener::EV_TYPE_BYTE_TWEENER_COMPLETED())
	{
        ByteTweener* tweener = static_cast<ByteTweener*>(evData);
		if (tweener)
            setState(tweener->getTweenedValue() > 128 ? LevelOverlay::FADE : LevelOverlay::GLOW);
    }
}

void LevelOverlay::advanceTime(float dt)
{
    if (getState() == LevelOverlay::IDLE)
        return;
    
    if (mRuneTweener)
        mRuneTweener->advanceTime(dt);
}
