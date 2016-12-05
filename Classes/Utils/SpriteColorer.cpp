
#include "SpriteColorer.h"
#include <Utils/Utils.h>
#include <Interfaces/IEventListener.h>
USING_NS_CC;

#pragma warning( disable : 4351 ) // new behavior: elements of array 'SpriteColorer::mColorParts' will be default initialized (changed in Visual C++ 2005) 
SpriteColorer::SpriteColorer(CCSprite* sprite, IEventListener* listener)
	: mSprite(NULL), mListener(listener), mColorParts(), mTweeners(), mIsComplete(false)
{
	CCAssert(sprite, "SpriteColorer requires non-null CCSprite to color.");
	mSprite = sprite;
    setAnimatableName("SpriteColorer");
}

SpriteColorer::~SpriteColorer(void)
{
    mSprite = NULL;
	mListener = NULL;
}

const ccColor3B SpriteColorer::getColor(void) const
{
	return ccc3(mColorParts[kRedIndex], mColorParts[kGreenIndex], mColorParts[kBlueIndex]);
}

void SpriteColorer::setColor(const ccColor3B& value)
{
	setComplete(true);
	updateColor(value);
}

void SpriteColorer::setColor(uint value)
{
	setColor(CMUtils::uint2color3B(value));
}

void SpriteColorer::animateColor(uint color, float duration, float delay)
{
	animateColor(CMUtils::uint2color3B(color), duration, delay);
}

void SpriteColorer::animateColor(const ccColor3B& color, float duration, float delay)
{
	const ccColor3B& spriteColor = mSprite->getColor();
	mTweeners[kRedIndex].reset(spriteColor.r, color.r, duration, delay);
	mTweeners[kGreenIndex].reset(spriteColor.g, color.g, duration, delay);
	mTweeners[kBlueIndex].reset(spriteColor.b, color.b, duration, delay);
	setComplete(false);
}

void SpriteColorer::updateColor(const ccColor3B& color)
{
	mColorParts[kRedIndex] = color.r;
	mColorParts[kGreenIndex] = color.g;
	mColorParts[kBlueIndex] = color.b;
	mSprite->setColor(color);
	
	if (mListener)
		mListener->onEvent(EV_TYPE_COLOR_DID_CHANGE(), this);
}

void SpriteColorer::advanceTime(float dt)
{
	if (isComplete())
        return;

	bool finishedColoring = true, didColorChange = false;
    for (int i = 0; i < 3; ++i)
    {
		ByteTweener* tweener = &mTweeners[i];
		tweener->advanceTime(dt);
		if (!tweener->isDelaying() && mColorParts[i] != tweener->getTweenedValue())
		{
			mColorParts[i] = tweener->getTweenedValue();
			didColorChange = true;
		}

        if (!tweener->isComplete())
            finishedColoring = false;
    }

	if (didColorChange)
		updateColor(getColor());
    setComplete(finishedColoring);
}
