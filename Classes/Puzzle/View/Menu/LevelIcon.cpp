
#include "LevelIcon.h"
#include <Puzzle/View/Menu/LevelMenu.h>
#include <Utils/Utils.h>
#include <Utils/LangFX.h>
USING_NS_CC;

//static const uint kBorderColor = 0xf3f3a8;
//static const int kOutlineIndex = 0;
//static const int kTextIndex = 1;

#pragma warning( disable : 4351 ) // new behavior: elements of array 'xyz' will be default initialized (changed in Visual C++ 2005) 
LevelIcon::LevelIcon(int category, int levelIndex, int levelID)
	:
	Prop(category),
	mLevelIndex(levelIndex),
	mLevelID(levelID),
    mLabelBaseScale(1.0f),
    mLevelLabel(NULL),
    mSolvedLabel(NULL),
    mBorder(NULL),
	mHighlight(NULL),
    mSolvedGlow(NULL),
	mLock(NULL),
	mBatch(NULL)
{
    
}

LevelIcon::~LevelIcon(void)
{
    CC_SAFE_RELEASE_NULL(mSolvedIcon);
    CC_SAFE_RELEASE_NULL(mSolvedGlow);
    CC_SAFE_RELEASE_NULL(mLevelLabel);
    CC_SAFE_RELEASE_NULL(mSolvedLabel);
    CC_SAFE_RELEASE_NULL(mBorder);
	CC_SAFE_RELEASE_NULL(mHighlight);
	CC_SAFE_RELEASE_NULL(mLock);
	CC_SAFE_RELEASE_NULL(mBatch);
}

bool LevelIcon::init(void)
{
	if (mBatch)
		return true;

	mBatch = new CCSpriteBatchNode();
	mBatch->initWithTexture(mScene->textureByName("menu-atlas"), 20);
	addChild(mBatch);

	mHighlight = new CCSprite();
	mHighlight->initWithSpriteFrame(mScene->spriteFrameByName("level-highlight"));
    mHighlight->setScaleX((75 + 256.0f) / mHighlight->boundingBox().size.width);
    mHighlight->setScaleY((75 + 265.0f) / mHighlight->boundingBox().size.height);
//    mHighlight->setScaleX(1.275f * 256.0f / mHighlight->boundingBox().size.width);
//    mHighlight->setScaleY(1.275f * 265.0f / mHighlight->boundingBox().size.height);
	//mHighlight->setScale(348.0f / mHighlight->boundingBox().size.width);
	//mHighlight->setColor(CMUtils::uint2color3B(0xffff4d));
    mHighlight->setColor(CMUtils::uint2color3B(LevelMenu::kLevelColors[mLevelIndex]));
	mHighlight->setVisible(false);
	mBatch->addChild(mHighlight);

	CCRect bounds = mHighlight->boundingBox();

	mBorder = CCSprite::createWithSpriteFrame(mScene->spriteFrameByName("level-icon-inner"));
    mBorder->retain();
	mBorder->setColor(CMUtils::uint2color3B(LevelMenu::kLevelColors[mLevelIndex]));
	mBatch->addChild(mBorder);

	bounds = CMUtils::unionRect(bounds, mBorder->boundingBox());
    
    char labelStr[33] = ""; memset(labelStr, ' ', 32 * sizeof(char));
    mLevelLabel = TextUtils::create(
                                    labelStr,
                                    32,
                                    kCCLabelAutomaticWidth,
                                    kCCTextAlignmentLeft,
                                    CCPointZero);
    mLabelBaseScale = mLevelLabel->getScale();
    mLevelLabel->setAnchorPoint(ccp(0.5f, 0.0f));
    mLevelLabel->setString(SceneController::localizeString(LevelMenu::kLevelNames[mLevelIndex]).c_str());
    mLevelLabel->retain();
    addChild(mLevelLabel);
    
    layoutLabel();

	bounds = CMUtils::unionRect(bounds, mLevelLabel->boundingBox());

	CCSprite* pinStripe = CCSprite::createWithSpriteFrame(mScene->spriteFrameByName("level-pin-stripe"));
	pinStripe->setPosition(ccp(
		mBorder->boundingBox().getMidX(),
		LangFX::getLevelIconYOffsets().val_0 - 2));
	pinStripe->setScaleX(0.9f * mBorder->boundingBox().size.width / pinStripe->boundingBox().size.width);
	mBatch->addChild(pinStripe);

	bounds = CMUtils::unionRect(bounds, pinStripe->boundingBox());

	CCSprite* levelIcon = CCSprite::createWithSpriteFrame(mScene->spriteFrameByName(LevelMenu::kLevelTextureNames[mLevelIndex]));
    levelIcon->setScale((mLevelIndex == LevelMenu::kNumLevels - 1 ? 1.0f : 1.0f)); // 0.9f : 0.925f));
	levelIcon->setPositionY((mLevelIndex == LevelMenu::kNumLevels - 1 ? 0 : 0)); // -4 : 0));
	addChild(levelIcon);

	bounds = CMUtils::unionRect(bounds, levelIcon->boundingBox());
    
    // Solved label
	CCPoint solvedPos = ccp(LangFX::getLevelIconSolvedOffsets().val_0, LangFX::getLevelIconSolvedOffsets().val_1); //ccp(-24, -93);
    mSolvedLabel = TextUtils::create(
                                     "0/6",
                                     28,
                                     kCCLabelAutomaticWidth,
                                     kCCTextAlignmentLeft,
                                     CCPointZero);
    mSolvedLabel->retain();
    mSolvedLabel->setPosition(solvedPos);
    addChild(mSolvedLabel);

	mSolvedIcon = CCSprite::createWithSpriteFrame(mScene->spriteFrameByName("menu-key"));
    mSolvedIcon->retain();
	mSolvedIcon->setScale(0.7f);
    mSolvedIcon->setPosition(ccp(
                                mSolvedLabel->boundingBox().getMaxX() + mSolvedIcon->boundingBox().size.width / 2,
                                mSolvedLabel->getPositionY() + LangFX::getLevelIconSolvedOffsets().val_2));
    addChild(mSolvedIcon);

	mLock = new CCSprite();
	mLock->initWithSpriteFrame(mScene->spriteFrameByName("level-lock"));
	mLock->setPositionY(-(mBorder->boundingBox().size.height - 0.875f * mLock->boundingBox().size.height) / 2);
	addChild(mLock);

	bounds = CMUtils::unionRect(bounds, mLock->boundingBox());
    
    
    // Override other bounds calcs
    bounds = mBorder->boundingBox();
    

	setContentSize(CCSizeMake(bounds.size.width, bounds.size.height));
	return true; // We'll know immediately if it fails anyway.
}

void LevelIcon::layoutLabel(void)
{
    if (mLevelLabel && mBorder)
    {
        mLevelLabel->setScale(mLabelBaseScale);
        mLevelLabel->setPosition(ccp(
                                     mBorder->boundingBox().getMidX(), // mBorder->boundingBox().getMinX() + 20,
                                     16 + LangFX::getLevelIconYOffsets().val_1));
        
        CCRect labelBounds = mLevelLabel->boundingBox();
        CCRect borderBounds = mBorder->boundingBox();
        
        float scaleMax = 0.85f;
        if (labelBounds.size.width > scaleMax * borderBounds.size.width)
        {
            mLevelLabel->setScale(mLevelLabel->getScale() * (scaleMax * borderBounds.size.width) / labelBounds.size.width);
            mLevelLabel->setPositionY(mLevelLabel->getPositionY() + 0.25f * (labelBounds.size.height - mLevelLabel->boundingBox().size.height));
        }
    }
}

void LevelIcon::localeDidChange(const char* fontKey, const char* FXFontKey)
{
    if (mLevelLabel)
    {
        mLevelLabel->setString("");
        mLevelLabel->setFntFile(fontKey);
        mLevelLabel->setString(SceneController::localizeString(LevelMenu::kLevelNames[mLevelIndex]).c_str());
        layoutLabel();
    }
    
    if (mSolvedLabel)
    {
        std::string solvedString = mSolvedLabel->getString();
        mSolvedLabel->setString("");
        mSolvedLabel->setFntFile(fontKey);
        mSolvedLabel->setString(solvedString.c_str());
    }
    
    if (mSolvedLabel && mSolvedIcon)
    {
        CCPoint solvedPos = ccp(LangFX::getLevelIconSolvedOffsets().val_0, LangFX::getLevelIconSolvedOffsets().val_1);
        mSolvedLabel->setPosition(solvedPos);
        mSolvedIcon->setPosition(ccp(
                                     mSolvedLabel->boundingBox().getMaxX() + mSolvedIcon->boundingBox().size.width / 2,
                                     mSolvedLabel->getPositionY() + LangFX::getLevelIconSolvedOffsets().val_2));
    }
}

void LevelIcon::enableHighlight(bool enable)
{
	if (mHighlight)
		mHighlight->setVisible(enable);
}

void LevelIcon::enableSolvedGlow(bool enable)
{
    if (mSolvedGlow)
        mSolvedGlow->removeFromParent();
    
    if (enable)
    {
        if (mSolvedGlow == NULL)
        {
            mSolvedGlow = CCSprite::createWithSpriteFrame(mScene->spriteFrameByName("menu-key-glow"));
            mSolvedGlow->retain();
            mSolvedGlow->setScale(0.7f);
            mSolvedGlow->setPosition(mSolvedIcon->getPosition());
            mSolvedGlow->setOpacity(0);
        }
        
        addChild(mSolvedGlow);
    }
}

CCRect LevelIcon::getVisibleBounds(void)
{
    if (mBorder)
    {
        return CCRectMake(0, 0, mBorder->boundingBox().size.width, mBorder->boundingBox().size.height);
    }
    else
        return CCRectZero;
}

CCPoint LevelIcon::getVisibleWorldCenter(void)
{
    if (mBorder)
        return convertToWorldSpace(mBorder->getPosition());
    else
        return CCPointZero;
}

CCPoint LevelIcon::getSolvedKeyWorldCenter(void)
{
    if (mSolvedIcon)
        return convertToWorldSpace(mSolvedIcon->getPosition());
    else
        return CCPointZero;
}

void LevelIcon::setPuzzlesSolved(int numSolved, int numPuzzles)
{
    mSolvedLabel->setString(CMUtils::strConcatVal("", numSolved).append(CMUtils::strConcatVal("/", numPuzzles)));
}

void LevelIcon::setLevelTextColor(uint color)
{
	if (mLevelLabel)
		mLevelLabel->setColor(CMUtils::uint2color3B(color));
}

void LevelIcon::setPuzzlesSolvedColor(uint color)
{
    if (mSolvedLabel)
		mSolvedLabel->setColor(CMUtils::uint2color3B(color));
}
