
#include "HUDCell.h"
#include <Puzzle/View/Playfield/HUD/PlayerHUD.h>
#include <Puzzle/Data/Player.h>
#include <Utils/Utils.h>
#include <Utils/LangFX.h>
USING_NS_CC;

HUDCell::HUDCell(void)
	:
	Prop(-1),
	mIcon(NULL),
	mLabel(NULL)
{

}

HUDCell::~HUDCell(void)
{
	//CC_SAFE_RELEASE(xyz);
}

HUDCell* HUDCell::create(bool autorelease)
{
	HUDCell *hudCell = new HUDCell();
	if (hudCell && hudCell->init())
    {
		if (autorelease)
			hudCell->autorelease();
        return hudCell;
    }
    CC_SAFE_DELETE(hudCell);
    return NULL;
}

bool HUDCell::init(void)
{
	if (mIcon)
		return true;

	mIcon = CCSprite::createWithSpriteFrame(mScene->spriteFrameByName("mirrored"));
	mIcon->setPositionX(mIcon->boundingBox().size.width / 2);
	addChild(mIcon);

	mLabel = TextUtils::create("", 48, kCCLabelAutomaticWidth, kCCTextAlignmentLeft, CCPointZero);
    mLabel->setAnchorPoint(ccp(0.0f, 0.5f));
	mLabel->setPosition(ccp(
		mIcon->getPositionX() + mIcon->boundingBox().size.width / 2,
		mIcon->getPositionY() + LangFX::getPlayerHUDSettings().val_0));
	mLabel->setColor(CMUtils::uint2color3B(PlayerHUD::kHUDGreen));
	addChild(mLabel);

	return true; // Obvious when it fails
}

void HUDCell::localeDidChange(const char* fontKey, const char* FXFontKey)
{
    if (mLabel)
    {
        mLabel->setString("");
        TextUtils::switchFntFile(fontKey, mLabel, false);
        
        if (mIcon)
            mLabel->setPositionY(mIcon->getPositionY() + LangFX::getPlayerHUDSettings().val_0);
    }
}

void HUDCell::setString(const char* value)
{
	if (mLabel)
		mLabel->setString(value);
}

uint HUDCell::getColor(void) const
{
	if (mLabel)
		return CMUtils::color3B2uint(mLabel->getColor());
	else
		return 0;
}

void HUDCell::setColor(uint value)
{
	if (mLabel)
		mLabel->setColor(CMUtils::uint2color3B(value));
}

void HUDCell::setIcon(uint evCode)
{
	if (mIcon == NULL || mEvCode == evCode)
		return;

	switch (evCode)
	{
		case Player::kValueMirroredMate:
			mIcon->setDisplayFrame(mScene->spriteFrameByName("mirrored"));
			break;
		case Player::kValueColorMagic:
			mIcon->setDisplayFrame(mScene->spriteFrameByName("color-magic"));
			break;
	}

	mEvCode = evCode;
}
