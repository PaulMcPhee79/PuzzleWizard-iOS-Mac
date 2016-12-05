
#include "MenuItem.h"
#include <Puzzle/View/Playfield/TilePiece.h>
#include <Utils/LangFX.h>
USING_NS_CC;

MenuItem::MenuItem(void)
:
mIgnoreLangOffsets(false),
mContents(NULL),
mSelectedIcon(NULL)
{

}

MenuItem::MenuItem(int fontSize)
	:
MenuButton(fontSize),
mIgnoreLangOffsets(false),
mContents(NULL),
mSelectedIcon(NULL)
{

}

MenuItem::~MenuItem(void)
{
    if (mSelectedIcon)
        mSelectedIcon->returnToPool();
    CC_SAFE_RELEASE_NULL(mSelectedIcon);
    CC_SAFE_RELEASE_NULL(mContents);
}

MenuItem* MenuItem::create(CCSpriteFrame* upFrame, CCSpriteFrame* downFrame, IEventListener* listener, bool autorelease)
{
	MenuItem *item = new MenuItem();
    if (upFrame && item && item->initWith(upFrame, downFrame, listener))
    {
        item->repositionActiveIcon();
		if (autorelease)
			item->autorelease();
        return item;
    }
    CC_SAFE_DELETE(item);
    return NULL;
}

MenuItem* MenuItem::create(const char* text, int fontSize, IEventListener* listener, bool autorelease)
{
	MenuItem *item = new MenuItem(fontSize);
    if (item && item->initWith(NULL, NULL, listener))
    {
        item->setString(text);
        item->repositionActiveIcon();
		item->setListener(listener);
		if (autorelease)
			item->autorelease();
        return item;
    }
    CC_SAFE_DELETE(item);
    return NULL;
}

bool MenuItem::initWith(CCSpriteFrame* upFrame, CCSpriteFrame* downFrame, IEventListener* listener)
{
    bool bRet = MenuButton::initWith(upFrame, downFrame, listener);
    
    if (bRet)
    {
        mContents = new CCNode();
        addBgContent(mContents);
        
#ifdef CHEEKY_DESKTOP
        mSelectedIcon = TileDecoration::getTileDecoration(TilePiece::kTDKPainter, TilePiece::kColorKeyGreen << 4);
        CC_SAFE_RETAIN(mSelectedIcon);
        mSelectedIcon->setVisible(isSelected());
        mSelectedIcon->enableUIMode(true);
        addBgContent(mSelectedIcon);
#endif
    }
    
    return bRet;
}

void MenuItem::localeDidChange(const char* fontKey, const char* FXFontKey)
{
    MenuButton::localeDidChange(fontKey, FXFontKey);
    repositionActiveIcon();
}

void MenuItem::setSelected(bool value)
{
    MenuButton::setSelected(value);
    
    if (mSelectedIcon)
        mSelectedIcon->setVisible(value);
}

void MenuItem::updateContentBounds(void)
{
    MenuButton::updateContentBounds();
    repositionActiveIcon();
}

void MenuItem::repositionActiveIcon(void)
{
    if (mSelectedIcon && mContents)
    {
        mSelectedIcon->removeFromParent();
        mSelectedIcon->setPosition(ccp(
                                       mContents->getPositionX() - (boundingBox().size.width / 2 + mSelectedIcon->boundingBox().size.width + 36),
                                       mContents->getPositionY() + (isIgnoreLangOffsetsEnabled() ? 0 : LangFX::getActiveIconYOffset())));
        addBgContent(mSelectedIcon);
    }
}

