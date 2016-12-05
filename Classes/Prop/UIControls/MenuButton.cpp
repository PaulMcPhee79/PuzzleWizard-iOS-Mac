
#include "MenuButton.h"
#include <Utils/Utils.h>
#include <Puzzle/Inputs/TouchPad.h>
USING_NS_CC;

static const float kDisabledOpacityFactor = 0.4f;
static const float kPressedScale = 0.9f;
static const float kTouchPadding = 50.0f;
static const int kNumChildScales = 3;

#pragma warning( disable : 4351 ) // new behavior: elements of array 'xyz' will be default initialized (changed in Visual C++ 2005) 
MenuButton::MenuButton(void)
	:
	Prop(-1),
    mSelected(false),
    mSelectedColor(0xffffff),
    mSelectedLabelColor(0xffffff),
    mDisabledOpacityFactor(kDisabledOpacityFactor),
    mScaleWhenPressed(kPressedScale),
    mChildScales(),
	mState(ENABLED),
	mEnabledOpacity(255),
    mFrameColors(),
    mTouchBounds(CCRectZero),
	mSprite(NULL),
	mSpriteFrames(),
	mBgNode(NULL),
	mLabelFontSize(32),
    mLabelColors(),
	mLabelAlignH(kCCTextAlignmentCenter),
	mLabelAlignV(kCCVerticalTextAlignmentCenter),
	mLabel(NULL),
    mColorables(NULL),
	mTouchCategory(CMTouches::TC_NONE),
	mListener(NULL)
{

}

MenuButton::MenuButton(int fontSize)
	:
	Prop(-1),
    mSelected(false),
    mSelectedColor(0xffffff),
    mSelectedLabelColor(0xffffff),
    mDisabledOpacityFactor(kDisabledOpacityFactor),
    mScaleWhenPressed(kPressedScale),
    mChildScales(),
	mState(ENABLED),
    mEnabledOpacity(255),
    mFrameColors(),
    mTouchBounds(CCRectZero),
	mSprite(NULL),
	mSpriteFrames(),
	mBgNode(NULL),
	mLabelFontSize(fontSize > 0 ? fontSize : 32),
    mLabelColors(),
	mLabelAlignH(kCCTextAlignmentCenter),
	mLabelAlignV(kCCVerticalTextAlignmentCenter),
	mLabel(NULL),
    mColorables(NULL),
    mTouchCategory(CMTouches::TC_NONE),
	mListener(NULL)
{
	
}

MenuButton::~MenuButton(void)
{
	mScene->deregisterForTouches(mTouchCategory, this);

	for (int i = 0; i < kNumDisplayFrames; ++i)
		CC_SAFE_RELEASE_NULL(mSpriteFrames[i]);

	CC_SAFE_RELEASE_NULL(mBgNode);
    CC_SAFE_DELETE(mColorables);
    
	mSprite = NULL;
	mLabel = NULL;
	mListener = NULL;
}

MenuButton* MenuButton::create(bool autorelease)
{
	MenuButton *button = new MenuButton();
	if (button && button->init())
    {
		if (autorelease)
			button->autorelease();
        return button;
    }
    CC_SAFE_DELETE(button);
    return NULL;
}

MenuButton* MenuButton::create(CCSpriteFrame* upFrame, CCSpriteFrame* downFrame, IEventListener* listener, bool autorelease)
{
	MenuButton *button = new MenuButton();
    if (upFrame && button && button->initWith(upFrame, downFrame, listener))
    {
		if (autorelease)
			button->autorelease();
        return button;
    }
    CC_SAFE_DELETE(button);
    return NULL;
}

MenuButton* MenuButton::create(const char* text, int fontSize, IEventListener* listener, bool autorelease)
{
	MenuButton *button = new MenuButton(fontSize);
    if (button && button->initWith(NULL, NULL, listener))
    {
        button->setString(text);
		button->setListener(listener);
		if (autorelease)
			button->autorelease();
        return button;
    }
    CC_SAFE_DELETE(button);
    return NULL;
}

bool MenuButton::initWith(CCSpriteFrame* upFrame, CCSpriteFrame* downFrame, IEventListener* listener)
{
	if (mSprite)
		return true;

    if (upFrame == NULL)
        upFrame = mScene->spriteFrameByName("clear");
	mSprite = CCSprite::createWithSpriteFrame(upFrame);

	bool bRet = mSprite != NULL;
	if (bRet)
	{
		mBgNode = new CCNode();
		addChild(mBgNode);
		addChild(mSprite);

		if (upFrame)
		{
			mSpriteFrames[kEnabledIndex] = upFrame;
			upFrame->retain();
			mSpriteFrames[kDisabledIndex] = upFrame;
			upFrame->retain();
            mSpriteFrames[kSelectedIndex] = upFrame;
			upFrame->retain();
		}

		CCSpriteFrame* pressedFrame = downFrame ? downFrame : upFrame;
		if (pressedFrame)
		{
			mSpriteFrames[kPressedIndex] = pressedFrame;
			pressedFrame->retain();
		}

		for (int i = kEnabledIndex; i <= kPressedIndex; ++i)
        {
			mLabelColors[i] = 0xffffff;
            mFrameColors[i] = 0xffffff;
        }
        
        for (int i = 0; i < kNumChildScales; ++i)
            mChildScales[i] = 1.0f;

		mListener = listener;
		updateContentBounds();
	}

	return bRet;
}

void MenuButton::localeDidChange(const char* fontKey, const char* FXFontKey)
{
    TextUtils::switchFntFile(fontKey, mLabel);
    updateContentBounds();
}

void MenuButton::setState(MenuButtonState value)
{
    if (value == mState)
        return;
    
	switch (value)
	{
		case MenuButton::ENABLED:
			setOpacityChildren(mEnabledOpacity);
			setUpScaleChildren();
			break;
		case MenuButton::DISABLED:
			setOpacityChildren((GLubyte)(mDisabledOpacityFactor * mEnabledOpacity));
			setUpScaleChildren();
			break;
		case MenuButton::PRESSED:
			setDownScaleChildren();
			break;
	}

	mState = value;
    updateDisplayFrame();
    updateContentColor();
}

int MenuButton::getIndexForState(MenuButtonState state)
{
    switch (state)
	{
		case MenuButton::DISABLED: return kDisabledIndex;
        case MenuButton::PRESSED: return kPressedIndex;
        case MenuButton::ENABLED:
        default:
            return kEnabledIndex;
	}
}

void MenuButton::setSelectedDisplayFrame(CCSpriteFrame* value)
{
    mSpriteFrames[kSelectedIndex] = value;
    updateDisplayFrame();
}

void MenuButton::enable(bool enable)
{
	if (enable)
	{
		if (getState() == MenuButton::DISABLED)
			setState(MenuButton::ENABLED);
	}
	else
	{
		if (getState() == MenuButton::ENABLED)
			setState(MenuButton::DISABLED);
	}
}

void MenuButton::depress(void)
{
	if (getState() == MenuButton::ENABLED)
	{
		setState(MenuButton::PRESSED);
		if (mListener)
			mListener->onEvent(EV_TYPE_PRESSED(), this);
	}
}

void MenuButton::raise(void)
{
	if (getState() == MenuButton::PRESSED)
	{
		setState(MenuButton::ENABLED);
		mScene->playSound("button");
		if (mListener)
			mListener->onEvent(EV_TYPE_RAISED(), this);
	}
}

void MenuButton::reset(void)
{
	if (getState() != MenuButton::DISABLED)
    {
		setState(MenuButton::ENABLED);
        if (mListener)
            mListener->onEvent(EV_TYPE_RESET(), this);
    }
}

void MenuButton::enforceOpacity(GLubyte opacity)
{
	switch (mState)
	{
		case MenuButton::ENABLED:
		case MenuButton::PRESSED:
			setOpacityChildren((GLubyte)((mEnabledOpacity / 255.0f) * opacity));
			break;
		case MenuButton::DISABLED:
			setOpacityChildren((GLubyte)(mDisabledOpacityFactor * (mEnabledOpacity / 255.0f) * opacity));
			break;
	}
}

CCLabelBMFont* MenuButton::createLabel(const char* text)
{
	return TextUtils::create(text, mLabelFontSize, kCCLabelAutomaticWidth, mLabelAlignH, CCPointZero);
}

void MenuButton::setUpScaleChildren(void)
{
    if (mSprite)
		mSprite->setScale(mChildScales[0]);
	if (mLabel)
		mLabel->setScale(mChildScales[1]);
	if (mBgNode)
		mBgNode->setScale(mChildScales[2]);
}

void MenuButton::setDownScaleChildren(void)
{
    float value = getScaleWhenPressed();
    
    if (mSprite)
    {
        mChildScales[0] = mSprite->getScale();
		mSprite->setScale(mChildScales[0] * value);
    }
	if (mLabel)
    {
        mChildScales[1] = mLabel->getScale();
		mLabel->setScale(mChildScales[1] * value);
    }
	if (mBgNode)
    {
        mChildScales[2] = mBgNode->getScale();
		mBgNode->setScale(mChildScales[2] * value);
    }
}

void MenuButton::setContentColor(uint value)
{
    setEnabledFrameColor(value);
    setPressedFrameColor(value);
    setDisabledFrameColor(value);
    setSelectedFrameColor(value);
}

void MenuButton::setString(const char* value)
{
	if (mLabel == NULL)
	{
		mLabel = createLabel(value);
		updateAlignment();
		addChild(mLabel);
	}
    else
    {
        mLabel->setString(SceneController::localizeString(value));
    }
    
    updateContentBounds();
}

void MenuButton::setFontSize(int value)
{
	CCAssert(value > 0, "MenuButton font size must be > 0");
	mLabelFontSize = value > 0 ? value : mLabelFontSize;

	if (mLabel)
	{
		CCLabelBMFont* oldLabel = mLabel;
		mLabel = createLabel(oldLabel->getString());
		updateAlignment();
		addChild(mLabel);
		oldLabel->removeFromParent();
		updateContentBounds();
	}
}

void MenuButton::setAlignmentH(CCTextAlignment alignment)
{
	mLabelAlignH = alignment;
	updateAlignment();
	updateContentBounds();
}

void MenuButton::setAlignmentV(CCVerticalTextAlignment alignment)
{
	mLabelAlignV = alignment;
	updateAlignment();
	updateContentBounds();
}

void MenuButton::setEnabledOpacity(GLubyte value)
{
	mEnabledOpacity = value;

	if (getState() == MenuButton::DISABLED)
		setOpacityChildren((GLubyte)(mDisabledOpacityFactor * mEnabledOpacity));
	else
		setOpacityChildren(mEnabledOpacity);
}

void MenuButton::setEnabledLabelColor(uint value)
{
	mLabelColors[kEnabledIndex] = value;
	updateContentColor();
}

void MenuButton::setDisabledLabelColor(uint value)
{
	mLabelColors[kDisabledIndex] = value;
	updateContentColor();
}

void MenuButton::setPressedLabelColor(uint value)
{
	mLabelColors[kPressedIndex] = value;
	updateContentColor();
}

void MenuButton::setSelectedLabelColor(uint value)
{
    mSelectedLabelColor = value;
    updateContentColor();
}

void MenuButton::setEnabledFrameColor(uint value)
{
    mFrameColors[kEnabledIndex] = value;
    updateContentColor();
}

void MenuButton::setDisabledFrameColor(uint value)
{
    mFrameColors[kDisabledIndex] = value;
    updateContentColor();
}

void MenuButton::setPressedFrameColor(uint value)
{
    mFrameColors[kPressedIndex] = value;
    updateContentColor();
}

void MenuButton::setSelectedFrameColor(uint value)
{
    mSelectedColor = value;
    updateContentColor();
}

void MenuButton::updateContentColor(void)
{
    if (isSelected() && getState() == MenuButton::ENABLED)
    {
        mSprite->setColor(CMUtils::uint2color3B(mSelectedColor));
        if (mLabel)
            mLabel->setColor(CMUtils::uint2color3B(mSelectedLabelColor));
        if (mColorables)
        {
            for (std::vector<CCRGBAProtocol*>::iterator it = mColorables->begin(); it != mColorables->end(); ++it)
                (*it)->setColor(CMUtils::uint2color3B(mSelectedLabelColor));
        }
    }
    else
    {
        int colorIndex = getIndexForState(getState());
        mSprite->setColor(CMUtils::uint2color3B(mFrameColors[colorIndex]));
        if (mLabel)
            mLabel->setColor(CMUtils::uint2color3B(mLabelColors[colorIndex]));
        if (mColorables)
        {
            for (std::vector<CCRGBAProtocol*>::iterator it = mColorables->begin(); it != mColorables->end(); ++it)
                (*it)->setColor(CMUtils::uint2color3B(mLabelColors[colorIndex]));
        }
    }
}

void MenuButton::updateDisplayFrame(void)
{
    if (isSelected() && getState() == MenuButton::ENABLED)
    {
        if (mSpriteFrames[kSelectedIndex])
            mSprite->setDisplayFrame(mSpriteFrames[kSelectedIndex]);
    }
    else
    {
        int spriteFrameIndex = getIndexForState(getState());
        if (mSpriteFrames[spriteFrameIndex])
            mSprite->setDisplayFrame(mSpriteFrames[spriteFrameIndex]);
    }
}

void MenuButton::setMinTouchSize(const CCSize& touchSize)
{
    if (mSprite && mSprite->boundingBox().size.width != 0 && mSprite->boundingBox().size.height != 0)
    {
        float prevScaleX = mSprite->getScaleX(), prevScaleY = mSprite->getScaleY();
        
        mSprite->setScaleX(touchSize.width / mSprite->boundingBox().size.width);
        mSprite->setScaleY(touchSize.height / mSprite->boundingBox().size.height);
        mTouchBounds = mSprite->boundingBox();
        
        mSprite->setScaleX(prevScaleX);
        mSprite->setScaleY(prevScaleY);
        updateContentBounds();
    }
}

void MenuButton::updateContentBounds(void)
{
	CCRect dfRect = mSprite ? mSprite->boundingBox() : CCRectZero;
	CCRect lblRect = mLabel ? mLabel->boundingBox() : CCRectZero;
	CCRect cbounds = CMUtils::unionRect(mTouchBounds, CMUtils::unionRect(dfRect, lblRect));
	setContentSize(cbounds.size);
}

void MenuButton::updateAlignment(void)
{
	if (mLabel == NULL)
		return;

	float offsetX = 0, offsetY = 0;

	switch (mLabelAlignH)
	{
		case kCCTextAlignmentLeft:
			offsetX = -mLabel->boundingBox().size.width / 2;
			break;
		case kCCTextAlignmentCenter:
			offsetX = 0;
			break;
		case kCCTextAlignmentRight:
			offsetX = mLabel->boundingBox().size.width / 2;
			break;
	}

	switch (mLabelAlignV)
	{
		case kCCVerticalTextAlignmentTop:
			offsetY = boundingBox().size.height / 2 - mLabel->boundingBox().size.height / 2;
			break;
		case kCCVerticalTextAlignmentCenter:
			offsetY = 0;
			break;
		case kCCVerticalTextAlignmentBottom:
			offsetY = -boundingBox().size.height / 2 + mLabel->boundingBox().size.height / 2;
			break;
	}

	mLabel->setPosition(ccp(offsetX, offsetY));
}

//void MenuButton::updateAlignment(void)
//{
//	if (mLabel == NULL)
//		return;
//
//	float offsetX = 0, offsetY = 0;
//
//	switch (mLabelAlignH)
//	{
//		case kCCTextAlignmentLeft:
//			offsetX = mLabel->boundingBox().size.width / 2;
//			break;
//		case kCCTextAlignmentCenter:
//			offsetX = boundingBox().size.width / 2;
//			break;
//		case kCCTextAlignmentRight:
//			offsetX = boundingBox().size.width - mLabel->boundingBox().size.width / 2;
//			break;
//	}
//
//	switch (mLabelAlignV)
//	{
//		case kCCVerticalTextAlignmentTop:
//			offsetY = boundingBox().size.height - mLabel->boundingBox().size.height / 2;
//			break;
//		case kCCVerticalTextAlignmentCenter:
//			offsetY = boundingBox().size.height / 2;
//			break;
//		case kCCVerticalTextAlignmentBottom:
//			offsetY = mLabel->boundingBox().size.height / 2;
//			break;
//	}
//
//	mLabel->setPosition(ccp(offsetX, offsetY));
//}

void MenuButton::addBgContent(CCNode* content)
{
	if (mBgNode && content)
        mBgNode->addChild(content);
}

void MenuButton::removeBgContent(CCNode* content)
{
	if (mBgNode && content)
        mBgNode->removeChild(content);
}

void MenuButton::addBgContent(CCNode* content, CCRGBAProtocol* colorable)
{
    if (mBgNode && content && colorable) // && (void*)content == (void*)colorable)
    {
        addBgContent(content);
        
        if (mColorables == NULL)
            mColorables = new std::vector<CCRGBAProtocol*>();
        if (CMUtils::contains(*mColorables, colorable) == false)
            mColorables->push_back(colorable);
    }
}

void MenuButton::removeBgContent(CCNode* content, CCRGBAProtocol* colorable)
{
    if (mBgNode && mColorables && content && colorable) //(void*)content == (void*)colorable)
    {
        removeBgContent(content);
        
        if (CMUtils::contains(*mColorables, colorable))
            CMUtils::erase(*mColorables, colorable);
    }
}

void MenuButton::enableTouch(bool enable, CMTouches::TouchCategory category)
{
	mScene->deregisterForTouches(category, this);
	
	if (enable)
	{
		mScene->registerForTouches(category, this);
		mTouchCategory = category;
	}
}

void MenuButton::onEvent(int evType, void* evData)
{
	CMTouches::TouchNotice* touchNotice = (CMTouches::TouchNotice*)evData;
	if (touchNotice == NULL)
		return;

	CCPoint touchPos = convertToNodeSpace(touchNotice->pos);	
	if (evType == TouchPad::EV_TYPE_TOUCH_BEGAN())
	{
		if (mSprite->boundingBox().containsPoint(touchPos))
		{
			touchNotice->retainFocus(this);
			depress();
		}
	}
	else if (evType == TouchPad::EV_TYPE_TOUCH_ENDED() || evType == TouchPad::EV_TYPE_TOUCH_CANCELLED())
		raise();
	else if (evType == TouchPad::EV_TYPE_TOUCH_MOVED())
	{
        CCRect cancelBounds = mSprite->boundingBox();
        cancelBounds.setRect(
                             cancelBounds.origin.x - kTouchPadding,
                             cancelBounds.origin.y - kTouchPadding,
                             cancelBounds.size.width + 2 * kTouchPadding,
                             cancelBounds.size.height + 2 * kTouchPadding);
		if (!cancelBounds.containsPoint(touchPos))
			reset();
	}
}
