
#include "IQSlider.h"
#include <Puzzle/View/PuzzleHelper.h>
#include <Puzzle/Inputs/TouchPad.h>
#include <Utils/Utils.h>
USING_NS_CC;

static const float kTouchPadding = 70.0f;

IQSlider::IQSlider(int min, int max, IEventListener* listener)
    :
    Prop(-1),
    mIQMinValue(min),
    mIQMaxValue(max),
    mIQValue(0),
    mSliderPosX(0),
    mSliderPosMinX(0),
    mSliderPosMaxX(0),
    mIqLabel(NULL),
    mIqTag(NULL),
    mIqRule(NULL),
    mIqTagNode(NULL),
    mListener(listener)
{

}

IQSlider::~IQSlider(void)
{
    CC_SAFE_RELEASE_NULL(mIqRule);
    CC_SAFE_RELEASE_NULL(mIqTag);
    CC_SAFE_RELEASE_NULL(mIqLabel);
    CC_SAFE_RELEASE_NULL(mIqTagNode);
    mListener = NULL;
}

IQSlider* IQSlider::create(int min, int max, IEventListener* listener, bool autorelease)
{
    
	IQSlider *slider = new IQSlider(min, max, listener);
    if (slider && slider->init())
    {
		if (autorelease)
			slider->autorelease();
        return slider;
    }
    CC_SAFE_DELETE(slider);
    return NULL;
}

bool IQSlider::init()
{
    if (mIqTagNode)
		return true;

    mIqRule = new CCSprite();
    mIqRule->initWithSpriteFrame(mScene->spriteFrameByName("level-pin-stripe"));
    
    mIqTag = new CCSprite();
    mIqTag->initWithSpriteFrame(mScene->spriteFrameByName("iq-tag"));
    mIqTag->setColor(CMUtils::uint2color3B(Puzzles::colorForIQ(mIQMinValue)));
    mIqTag->setScaleX(1.165f);
    
    mIqLabel = TextUtils::createCommonLGE("000\n000", 26, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
    mIqLabel->setString(CMUtils::strConcatVal("IQ\n", mIQMinValue).c_str());
    mIqLabel->setPosition(ccp(
                              mIqTag->getPositionX() - 8,
                              mIqTag->getPositionY() + (mIqTag->boundingBox().size.height - mIqLabel->boundingBox().size.height) / 4));
    mIqLabel->setColor(CMUtils::uint2color3B(0xffffff));
    
    mIqTagNode = new CCNode();
    mIqTagNode->setPosition(ccp(0, 0));
    //mIqTagNode->setAnchorPoint(ccp(0, 1));
    mIqTagNode->setScaleX(1.0f);
    mIqTagNode->setScaleY(1.33f);
    mIqTagNode->addChild(mIqRule);
    mIqTagNode->addChild(mIqTag);
    mIqTagNode->addChild(mIqLabel);
    
    CCRect cbounds = CMUtils::unionRect(mIqRule->boundingBox(), mIqTag->boundingBox());
	mIqTagNode->setContentSize(cbounds.size);
    addChild(mIqTagNode);
    
    mSliderPosMinX = -cbounds.size.width / 2;
    mSliderPosMaxX = cbounds.size.width / 2;
    
    setIQValue(mIQMinValue);
    
	return true;
}

CCSize IQSlider::getSliderSize(void) const
{
    if (mIqTagNode)
        return mIqTagNode->getContentSize();
    else
        return CCSizeMake(1, 1);
}

void IQSlider::setIQValue(int value)
{
    int prevIQValue = mIQValue;
    setIQValueInternal(value);
    
    if (mIQValue != prevIQValue)
        moveSliderTo(IQValue2SliderPos(mIQValue));
}

void IQSlider::setIQValueInternal(int value)
{
    int prevIQValue = mIQValue;
    mIQValue = MIN(MAX(mIQMinValue, value), mIQMaxValue);
    
    if (mIQValue != prevIQValue)
    {
        int intervalValue = getIQIntervalValue();
    
        if (mIqTag)
            mIqTag->setColor(CMUtils::uint2color3B(Puzzles::colorForIQ(intervalValue)));
        if (mIqLabel)
            mIqLabel->setString(CMUtils::strConcatVal("IQ\n", intervalValue).c_str());
    
        if (mListener)
            mListener->onEvent(EV_TYPE_IQ_CHANGED(), this);
    }
}

int IQSlider::sliderPos2IQValue(float value)
{
    float sliderRange = getSliderPosRange();
    float normalizedValue = value - mSliderPosMinX;
    
    int iqValue = mIQMinValue;
    if (sliderRange > 0)
    {
        float sliderPosPercentage = normalizedValue / (float)sliderRange;
        iqValue = mIQMinValue + (int)(sliderPosPercentage * getIQRange());
    }
    return iqValue;
}

float IQSlider::IQValue2SliderPos(int value)
{
    int iqRange = getIQRange();
    int normalizedValue = value - mIQMinValue;
    
    float sliderPos = mSliderPosMinX;
    if (iqRange > 0)
    {
        float iqPercentage = normalizedValue / (float)iqRange;
        sliderPos = mSliderPosMinX + iqPercentage * getSliderPosRange();
    }
    return sliderPos;
}

void IQSlider::moveSliderTo(float value)
{
    if (mIqRule && mIqTag && mIqLabel)
    {
        mSliderPosX = MIN(MAX(-mIqRule->boundingBox().size.width / 2, value), mIqRule->boundingBox().size.width / 2);
        mIqTag->setPositionX(mSliderPosX);
        mIqLabel->setPositionX(mIqTag->getPositionX() - 8);
    }
}

bool IQSlider::didTouchSliderKnob(const CCPoint& touchPos)
{
    if (mIqTag != NULL)
    {
        CCPoint pos = this->convertToNodeSpace(touchPos);
        CCRect cancelBounds = CMUtils::boundsInSpace(this, mIqTag);
        cancelBounds.setRect(
                             cancelBounds.origin.x - kTouchPadding,
                             cancelBounds.origin.y - kTouchPadding,
                             cancelBounds.size.width + 2 * kTouchPadding,
                             cancelBounds.size.height + 2 * kTouchPadding);
        if (cancelBounds.containsPoint(pos))
            return true;
    }
    
    return false;
}

void IQSlider::onEvent(int evType, void* evData)
{
    if (evType == TouchPad::EV_TYPE_TOUCH_BEGAN() || evType == TouchPad::EV_TYPE_TOUCH_MOVED() ||
		evType == TouchPad::EV_TYPE_TOUCH_ENDED() || evType == TouchPad::EV_TYPE_TOUCH_CANCELLED())
	{
		onTouch(evType, evData);
	}
}

void IQSlider::onTouch(int evType, void* evData)
{
	CMTouches::TouchNotice* touchNotice = (CMTouches::TouchNotice*)evData;
	if (touchNotice == NULL)
		return;
    
	if (evType == TouchPad::EV_TYPE_TOUCH_MOVED())
	{
		if (didTouchSliderKnob(touchNotice->pos))
        {
            CCPoint touchPos = convertToNodeSpace(touchNotice->pos);
            CCPoint prevTouchPos = convertToNodeSpace(touchNotice->prevPos);
            CCPoint deltaPos = ccp(touchPos.x - prevTouchPos.x, touchPos.y - prevTouchPos.y);
            float deltaX = deltaPos.x;
            if (deltaX != 0)
            {
                moveSliderBy(deltaX);
                setIQValueInternal(sliderPos2IQValue(getSliderPosX()));
            }
        }
	}
}
