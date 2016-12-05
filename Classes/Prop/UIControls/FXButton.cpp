
#include "FXButton.h"
#include <Puzzle/View/Playfield/TileDecoration.h>
#include <Puzzle/View/Playfield/TilePiece.h>
#include <Utils/Utils.h>
USING_NS_CC;

FXButton::FXButton(TextUtils::CMFontType fontType)
    :
    Prop(-1),
    mFontType(fontType),
    mSelectedColor(0xffffff),
    mUnselectedColor(0xffffff),
    mContentBounds(CCRectZero),
    mLabel(NULL),
    mSelectedArrow(NULL),
    mButtonProxy(NULL),
    mContents(NULL),
    mListener(NULL)
{

}

FXButton::~FXButton(void)
{
    if (mSelectedArrow)
        mSelectedArrow->returnToPool();
    CC_SAFE_RELEASE_NULL(mSelectedArrow);
    
    CC_SAFE_RELEASE_NULL(mLabel);
    CC_SAFE_RELEASE_NULL(mButtonProxy);
    CC_SAFE_RELEASE_NULL(mContents);
	mListener = NULL;
}

FXButton* FXButton::create(TextUtils::CMFontType fontType, const char* text, int fontSize, IEventListener* listener, bool autorelease)
{
	FXButton *button = new FXButton(fontType);
    if (fontSize > 0 && button && button->initWith(text, fontSize, listener))
    {
		if (autorelease)
			button->autorelease();
        return button;
    }
    CC_SAFE_DELETE(button);
    return NULL;
}

bool FXButton::initWith(const char* text, int fontSize, IEventListener* listener)
{
    if (mContents)
		return true;
    
    bool bRet = Prop::init();
    
    do {
        mContents = new CCNode();
        if (!(bRet = bRet && mContents)) break;
        addChild(mContents);
        
        mLabel = TextUtils::create(mFontType, text, fontSize, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
        if (!(bRet = bRet && mLabel)) break;
        CC_SAFE_RETAIN(mLabel);
        mLabel->setColor(CMUtils::uint2color3B(mSelectedColor));
        
        mButtonProxy = MenuButton::create(mScene->spriteFrameByName("clear"), NULL, this, false);
        
        mSelectedArrow = TileDecoration::getTileDecoration(TilePiece::kTDKPainter, TilePiece::kColorKeyGreen << 4);
        if (!(bRet = bRet && mSelectedArrow)) break;
        CC_SAFE_RETAIN(mSelectedArrow);
        mSelectedArrow->enableUIMode(true);
        mSelectedArrow->setVisible(false);
        
        mContents->addChild(mLabel);
        mContents->addChild(mSelectedArrow);
        mContents->addChild(mButtonProxy);
        
        mListener = listener;
        layoutContents();
        updateContentBounds();
        repositionSelectedArrow();
        updateContentColor();
    } while (false);
    
	return bRet;
}

void FXButton::localeDidChange(const char* fontKey, const char* FXFontKey)
{
    TextUtils::switchFntFile(mFontType == TextUtils::FONT_FX ? FXFontKey : fontKey, mLabel);
    layoutContents();
    updateContentBounds();
    repositionSelectedArrow();
}
                         
void FXButton::setSelected(bool value)
{
    if (mButtonProxy)
        mButtonProxy->setSelected(value);
    if (mSelectedArrow)
        mSelectedArrow->setVisible(value);
}

void FXButton::setString(const char* value)
{
    if (mLabel)
    {
        mLabel->setString(value);
        layoutContents();
        updateContentBounds();
        repositionSelectedArrow();
    }
}

void FXButton::repositionSelectedArrow(void)
{
    if (mSelectedArrow)
    {
        mSelectedArrow->setPosition(ccp(
                                        mContentBounds.getMinX() - mSelectedArrow->getScale() * (mSelectedArrow->boundingBox().size.width + 36),
                                        mContentBounds.getMidY() + 0.1f * mContentBounds.size.height));
    }
}

void FXButton::layoutContents(void) { /* Do nothing */ }

void FXButton::setSelectedColor(uint value)
{
    mSelectedColor = value;
    updateContentColor();
}

void FXButton::setUnselectedColor(uint value)
{
    mUnselectedColor = value;
    updateContentColor();
}

void FXButton::setSelectedArrowScale(float value)
{
    if (mSelectedArrow)
    {
        mSelectedArrow->setScale(value);
        repositionSelectedArrow();
    }
}

void FXButton::updateContentColor(void)
{
    if (mLabel == NULL)
        return;
#ifdef CHEEKY_MOBILE
    if (isPressed())
        mLabel->setColor(CMUtils::uint2color3B(mSelectedColor));
    else
        mLabel->setColor(CMUtils::uint2color3B(mUnselectedColor));
#endif
    
#ifdef CHEEKY_DESKTOP
    if (isSelected())
        mLabel->setColor(CMUtils::uint2color3B(mSelectedColor));
    else
        mLabel->setColor(CMUtils::uint2color3B(mUnselectedColor));
#endif
}

void FXButton::updateContentBounds(void)
{
    if (mLabel && mButtonProxy)
    {
        mContentBounds = mLabel->boundingBox();
        mButtonProxy->setMinTouchSize(mContentBounds.size);
        setContentSize(mContentBounds.size);
    }
}

void FXButton::onEvent(int evType, void* evData)
{
    if (evType == MenuButton::EV_TYPE_PRESSED())
    {
#ifdef CHEEKY_MOBILE
        updateContentColor();
#endif
        mContents->setScale(0.9f);
    }
    else if (evType == MenuButton::EV_TYPE_RAISED())
    {
#ifdef CHEEKY_MOBILE
        updateContentColor();
#endif
        mContents->setScale(1.0f);
    }
    else if (evType == MenuButton::EV_TYPE_RESET())
    {
#ifdef CHEEKY_MOBILE
        updateContentColor();
#endif
        mContents->setScale(1.0f);
    }
#ifdef CHEEKY_DESKTOP
    else if (evType == MenuButton::EV_TYPE_SELECTED())
    {
        updateContentColor();
    }
    else if (evType == MenuButton::EV_TYPE_UNSELECTED())
    {
        updateContentColor();
    }
#endif
    
    if (mListener)
        mListener->onEvent(evType, this);
}
