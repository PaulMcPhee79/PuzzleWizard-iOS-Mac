
#include "IconButton.h"
#include <Puzzle/View/Playfield/TileDecoration.h>
#include <Utils/Utils.h>
USING_NS_CC;

IconButton::IconButton(TextUtils::CMFontType fontType)
    :
    FXButton(fontType),
    mSelectedIconColor(0xffffff),
    mUnselectedIconColor(0xffffff),
    mIconOffset(CCSizeZero),
    mIcon(NULL)
{

}

IconButton::~IconButton(void)
{

}

IconButton* IconButton::create(TextUtils::CMFontType fontType, const char* text, int fontSize, CCSpriteFrame* iconFrame, IEventListener* listener, bool autorelease)
{
    
	IconButton *button = new IconButton(fontType);
    if (iconFrame && fontSize > 0 && button && button->initWith(text, fontSize, iconFrame, listener))
    {
		if (autorelease)
			button->autorelease();
        return button;
    }
    CC_SAFE_DELETE(button);
    return NULL;
}

bool IconButton::initWith(const char* text, int fontSize, CCSpriteFrame* iconFrame, IEventListener* listener)
{
    if (mIcon)
		return true;
    
    bool bRet = FXButton::initWith(text, fontSize, listener);
    
    if (bRet)
    {
        mSelectedIconColor = 0xffffff;
        mUnselectedIconColor = 0xffe611; // 0x0de60c;
        
        if (iconFrame == NULL)
            iconFrame = mScene->spriteFrameByName("clear");
        mIcon = CCSprite::createWithSpriteFrame(iconFrame);
        
        bRet = mIcon != NULL;
        if (bRet)
        {
            mContents->addChild(mIcon);
            layoutContents();
            updateContentBounds();
            repositionSelectedArrow();
            updateContentColor();
        }
    }
    
	return bRet;
}

CCSize IconButton::getIconSize(void) const
{
    if (mIcon)
        return mIcon->boundingBox().size;
    else
        return CCSizeMake(1, 1);
}

void IconButton::repositionSelectedArrow(void)
{
    // Ignore base version
    
    if (mSelectedArrow)
    {
        mSelectedArrow->setPosition(ccp(
                                        mContentBounds.getMinX() - (mSelectedArrow->boundingBox().size.width + 36),
                                        mContentBounds.getMidY()));
    }
}

void IconButton::layoutContents(void)
{
    // Ignore base version
    if (mIcon == NULL || mLabel == NULL)
        return;
    
    std::string labelStr = mLabel->getString();
    std::string trimmedStr = CMUtils::trim(labelStr);
    mLabel->setString(trimmedStr.c_str());
    
    mIcon->setPosition(ccp(mIcon->boundingBox().size.width / 2 + mIconOffset.x, 0));
    mLabel->setPosition(ccp(mIcon->getPositionX() + (mIcon->boundingBox().size.width + mLabel->boundingBox().size.width) / 2, mIcon->getPositionY()));
    
    CCRect cbounds = CMUtils::unionRect(mIcon->boundingBox(), mLabel->boundingBox());
    mIcon->setPositionX(mIcon->getPositionX() - cbounds.size.width / 2 + mIconOffset.x);
    mLabel->setPositionX(mLabel->getPositionX() - cbounds.size.width / 2);
    mIcon->setPositionY(mIconOffset.y);
    
    mLabel->setString(labelStr.c_str());
}

void IconButton::setSelectedColors(uint iconColor, uint fontColor)
{
    mSelectedIconColor = iconColor;
    setSelectedColor(fontColor);
    updateContentColor();
}

void IconButton::setUnselectedColors(uint iconColor, uint fontColor)
{
    mUnselectedIconColor = iconColor;
    setUnselectedColor(fontColor);
}

void IconButton::setIconOffset(const CCPoint& offset)
{
    mIconOffset = offset;
    layoutContents();
    updateContentBounds();
    repositionSelectedArrow();
}

void IconButton::updateContentColor(void)
{
    FXButton::updateContentColor();
    
    if (mIcon == NULL)
        return;
    
#ifdef CHEEKY_MOBILE
    if (isPressed())
        mIcon->setColor(CMUtils::uint2color3B(mSelectedIconColor));
    else
        mIcon->setColor(CMUtils::uint2color3B(mUnselectedIconColor));
#endif
    
#ifdef CHEEKY_DESKTOP
    if (isSelected())
        mIcon->setColor(CMUtils::uint2color3B(mSelectedIconColor));
    else
        mIcon->setColor(CMUtils::uint2color3B(mUnselectedIconColor));
#endif
}

void IconButton::updateContentBounds(void)
{
    // Ignore base version
    if (mIcon && mLabel && mButtonProxy)
    {
        mContentBounds = CMUtils::unionRect(mIcon->boundingBox(), mLabel->boundingBox());
        mButtonProxy->setMinTouchSize(mContentBounds.size);
        setContentSize(mContentBounds.size);
    }
}
