


#include "PuzzlePageEntry.h"
#include <Puzzle/View/Menu/PuzzlePage.h>
#include <Puzzle/View/Playfield/PuzzleBoard.h>
#include <Puzzle/View/Playfield/TilePiece.h>
#include <Puzzle/View/Playfield/TileDecoration.h>
#include <Utils/Utils.h>
#include <Utils/CMTypes.h>
#include <Utils/LangFX.h>
USING_NS_CC;

static const float kBoardShadowOffsetX = 6.0f;
static const float kBoardShadowOffsetY = -18.0f;

static const float kIqTagShadowOffsetX = 4.0f;
static const float kIqTagShadowOffsetY = -6.0f;

static const float kLabelWidthJP = 500.0f;

PuzzlePageEntry::PuzzlePageEntry(PuzzleBoard* board)
:
Prop(-1),
mIsPopulated(true),
mIsLocked(true),
mIsSolved(false),
mIsHighlighted(false),
mIsLabelShrunkToFit(false),
mHighlightColor(0xffffff),
mHighlightOffsetY(0),
mLabelBaseScale(1),
mLabelWidthMax(-1),
mLock(NULL),
mSolved(NULL),
mSolvedContainer(NULL),
mHighlightedIcon(NULL),
mBoard(NULL),
mBoardShadow(NULL),
mIqTagShadow(NULL),
mBoardContainer(NULL),
mLabel(NULL)
{
	CCAssert(board, "PuzzlePageEntry requires a non-NULL PuzzleBoard.");
	board->retain();
	mBoard = board;
}

PuzzlePageEntry::~PuzzlePageEntry(void)
{
    if (mHighlightedIcon)
        mHighlightedIcon->returnToPool();
    CC_SAFE_RELEASE_NULL(mHighlightedIcon);
	CC_SAFE_RELEASE_NULL(mLock);
	CC_SAFE_RELEASE_NULL(mSolved);
    CC_SAFE_RELEASE_NULL(mSolvedContainer);
	CC_SAFE_RELEASE_NULL(mBoard);
    CC_SAFE_RELEASE_NULL(mBoardShadow);
    CC_SAFE_RELEASE_NULL(mIqTagShadow);
	CC_SAFE_RELEASE_NULL(mLabel);
    CC_SAFE_RELEASE_NULL(mBoardContainer);
}

bool PuzzlePageEntry::init(void)
{
    LangFX::PuzzlePageSettings settings = PuzzlePage::getSettings();
    mLabelWidthMax = 1.2f * settings.puzzleBoardWidth;
    
    mBoardShadow = new CCSprite();
    mBoardShadow->initWithSpriteFrame(mScene->spriteFrameByName("shadow-rect"));
    mBoardShadow->setColor(CMUtils::uint2color3B(0));
    
    mIqTagShadow = new CCSprite();
    mIqTagShadow->initWithSpriteFrame(mScene->spriteFrameByName("iq-tag-shadow"));
    mIqTagShadow->setFlipX(true);
    mIqTagShadow->setColor(CMUtils::uint2color3B(0));
    
	mBoardContainer = CCNode::create();
    mBoardContainer->retain();
	mBoardContainer->addChild(mBoard);
    
    char labelStr[33] = ""; memset(labelStr, ' ', 32 * sizeof(char));
    mLabel = TextUtils::create(labelStr, 31, mLabelWidthMax, kCCTextAlignmentCenter, CCPointZero);
	mLabel->retain();
    //mLabel->increaseAtlasCapacity();
    mLabel->setString(" ");
    mLabel->setAnchorPoint(ccp(0.5f, 1.0f));
    mLabelBaseScale = mLabel->getScaleX();
    
    mLock = new CCSprite();
	mLock->initWithSpriteFrame(mScene->spriteFrameByName("level-lock"));
    mLock->setVisible(isLocked());
    
    mSolved = new CCSprite();
	mSolved->initWithSpriteFrame(mScene->spriteFrameByName("puzzle-menu-key"));
    mSolved->setVisible(isSolved());
    
    mSolvedContainer = new CCNode();
    mSolvedContainer->init();
    //mSolvedContainer->setScaleX(-1);
    mSolvedContainer->addChild(mSolved);
    
	resizeBoard();
	setIsPopulated(false);
    
    mHighlightOffsetY = -mLabel->boundingBox().size.height / 2;
    
#ifdef CHEEKY_DESKTOP
    mHighlightedIcon = TileDecoration::getTileDecoration(TilePiece::kTDKPainter, TilePiece::kColorKeyGreen << 4);
    CC_SAFE_RETAIN(mHighlightedIcon);
    mHighlightedIcon->setPosition(ccp(
                                      mLabel->getPositionX() - (0.5f * mLabel->boundingBox().size.width + mHighlightedIcon->boundingBox().size.width),
                                      mLabel->getPositionY() + mHighlightOffsetY + LangFX::getActiveIconYOffset()));
    mHighlightedIcon->setVisible(isHighlighted());
    mHighlightedIcon->enableUIMode(true);
#endif
    
    
    addChild(mBoardShadow);
    addChild(mIqTagShadow);
	addChild(mBoardContainer);
	addChild(mLabel);
    if (mHighlightedIcon)
        addChild(mHighlightedIcon);
    addChild(mLock);
    addChild(mSolvedContainer);
    
    vec2 boardDims = mBoard->getScaledBoardDimensions();
	CCRect bounds = CCRectMake(0, 0, boardDims.x * mBoardContainer->getScaleX(), boardDims.y * mBoardContainer->getScaleY());
	bounds = CMUtils::unionRect(bounds, mLabel->boundingBox());
	setContentSize(bounds.size);
    
	return true; // Obvious when it fails
}

void PuzzlePageEntry::setIsHighlighted(bool value)
{
	if (mIsHighlighted != value)
    {
        if (mLabel)
			mLabel->setColor(value ? CMUtils::uint2color3B(mHighlightColor) : CMUtils::uint2color3B(0xffffff));
        
		if (mBoard)
			mBoard->enableHighlight(value);
        
        mIsHighlighted = value;
        
        if (mHighlightedIcon)
        {
            if (value)
                repositionHighlightedIcon();
            mHighlightedIcon->setVisible(value);
        }
    }
}

void PuzzlePageEntry::setHighlightColor(uint value)
{
	mHighlightColor = value;
    
	if (mLabel)
		mLabel->setColor(isHighlighted() ? CMUtils::uint2color3B(mHighlightColor) : CMUtils::uint2color3B(0xffffff));
    
	if (mBoard)
		mBoard->setHighlightColor(CMUtils::uint2color3B(value));
}

void PuzzlePageEntry::resizeBoard(void)
{
	if (mBoardContainer && mBoard)
    {
        vec2 boardDims = mBoard->getBoardDimensions();
        // 44.0f = Half-tile width from overhang
        // 36.0f = Half-tile height from overhang
        //mBoard->setAnchorPoint(ccp(0.75f * (boardDims.x / (boardDims.x + 44.0f)), 0.75f * (boardDims.y / (boardDims.y + 36.0f))));
        mBoard->setAnchorPoint(ccp(0.75f * (boardDims.x / (boardDims.x + 44.0f)), 1.0f));
        
        CCPoint anchorPoint = mBoard->getAnchorPoint();
        //mBoard->setPosition(ccp((1.0f - (0.75f - anchorPoint.x)) * -boardDims.x / 2, (1.0f - (0.75f - anchorPoint.y)) * -boardDims.y / 2));
        mBoard->setPosition(ccp((1.0f - (0.75f - anchorPoint.x)) * -boardDims.x / 2, -boardDims.y));
        
        vec2 scaledBoardDims = mBoard->getScaledBoardDimensions();
        LangFX::PuzzlePageSettings settings = PuzzlePage::getSettings();
        mBoardContainer->setScaleX(settings.puzzleBoardWidth / scaledBoardDims.x);
        mBoardContainer->setScaleY(settings.puzzleBoardHeight / scaledBoardDims.y);
        
        CCPoint maxBoardDims = LangFX::getMaxPuzzlePageBoardDimensions();
//        mBoardContainer->setPosition(ccp(maxBoardDims.x / 2,
//                                         maxBoardDims.y / 2));
        mBoardContainer->setPosition(ccp(maxBoardDims.x / 2,
                                         maxBoardDims.y));
        
        CCPoint boardOrigin = mBoard->getPosition();
        boardOrigin.x -= 44.0f;
        boardOrigin.y -= 36.0f;
        boardOrigin = convertToNodeSpace(mBoardContainer->convertToWorldSpace(boardOrigin));
        
        if (mBoardShadow)
        {
            mBoardShadow->setScale(1.0f);
            vec2 shadowScale = cmv2(settings.puzzleBoardWidth / mBoardShadow->boundingBox().size.width,
                                    settings.puzzleBoardHeight / mBoardShadow->boundingBox().size.height);
            mBoardShadow->setPosition(ccp(boardOrigin.x + settings.puzzleBoardWidth / 2 + kBoardShadowOffsetX,
                                          boardOrigin.y + settings.puzzleBoardHeight / 2 + kBoardShadowOffsetY));
            mBoardShadow->setScaleX(1.15f * shadowScale.x);
            mBoardShadow->setScaleY(1.2f * shadowScale.y);
        }
        
        if (mIqTagShadow)
        {
            CCRect iqTagWorldBounds = mBoard->getIqTagGlobalBounds();
            
            CCPoint btmLeft = ccp(iqTagWorldBounds.getMinX(),
                                  iqTagWorldBounds.getMinY());
            btmLeft = convertToNodeSpace(btmLeft);
            
            CCPoint topRight = ccp(iqTagWorldBounds.getMaxX(),
                                   iqTagWorldBounds.getMaxY());
            topRight = convertToNodeSpace(topRight);
            
            CCRect iqTagLocalBounds = CCRectMake(btmLeft.x,
                                                 btmLeft.y,
                                                 topRight.x - btmLeft.x,
                                                 topRight.y - btmLeft.y);
            
            mIqTagShadow->setPosition(ccp(iqTagLocalBounds.getMidX() + kIqTagShadowOffsetX,
                                          iqTagLocalBounds.getMidY() + kIqTagShadowOffsetY));
            mIqTagShadow->setScale(1.0f);
            mIqTagShadow->setScaleX(1.25f * iqTagLocalBounds.size.width / mIqTagShadow->boundingBox().size.width);
            mIqTagShadow->setScaleY(1.25f * iqTagLocalBounds.size.height / mIqTagShadow->boundingBox().size.height);
        }
        
        if (mLabel)
        {
            mLabel->setPosition(ccp(boardOrigin.x + settings.puzzleBoardWidth / 2,
            						boardOrigin.y + 1.0f * mHighlightOffsetY + LangFX::getPuzzleLabelYOffset() - getShrunkLabelSizeDiff().height / 2));
        }
        
        if (mLock)
        {
            mLock->setPosition(ccp(
                                   boardOrigin.x + settings.puzzleBoardWidth / 2,
                                   boardOrigin.y + settings.puzzleBoardHeight / 2));
            mLock->setScale(settings.puzzleBoardWidth / maxBoardDims.x);
        }
        
        if (mSolved)
        {
//            mSolved->setScale(1.0f);
//            mSolvedContainer->setPosition(ccp(
//                                              boardOrigin.x + settings.puzzleBoardWidth - mSolved->boundingBox().size.width / 10,
//                                              boardOrigin.y + settings.puzzleBoardHeight));
//            mSolved->setScale(settings.puzzleBoardWidth / maxBoardDims.x);
            
            mSolvedContainer->setPosition(ccp(
                                              boardOrigin.x - mSolved->boundingBox().size.width / 4,
                                              boardOrigin.y + settings.puzzleBoardHeight));
        }
        
        CCRect bounds = CCRectMake(0, 0, boardDims.x * mBoardContainer->getScaleX(), boardDims.y * mBoardContainer->getScaleY());
        if (mLabel)
            bounds = CMUtils::unionRect(bounds, mLabel->boundingBox());
        setContentSize(bounds.size);
    }
}

const char* PuzzlePageEntry::getString(void)
{
	return mLabel ? mLabel->getString() : NULL;
}

void PuzzlePageEntry::setString(const char* value)
{
	if (mLabel)
    {
        mIsLabelShrunkToFit = false;
        Localizer::LocaleType locale = mScene->getLocale();
        
        if (locale == Localizer::JP)
        {
            mLabel->setWidth(kLabelWidthJP);
            mLabel->setString(value);
            resizeLabel();
            
            CCRect labelBounds = mLabel->boundingBox();
            if (labelBounds.size.width > mLabelWidthMax)
            {
                mIsLabelShrunkToFit = true;
                resizeLabel();
            }
        }
        else
        {
            mLabel->setWidth(mLabelWidthMax);
            mLabel->setString(value);
            resizeLabel();
        }
        
        repositionHighlightedIcon();
    }
}

void PuzzlePageEntry::resizeLabel(void)
{
    mLabel->setScale(mLabelBaseScale);
    
    if (mIsLabelShrunkToFit)
    {
        float scaleMax = 1.0f;
        CCRect labelBounds = mLabel->boundingBox();
        if (labelBounds.size.width > scaleMax * mLabelWidthMax)
        {
            float scaler = (scaleMax * mLabelWidthMax) / labelBounds.size.width;
            mLabel->setScale(mLabelBaseScale * scaler);
        }
    }
}

void PuzzlePageEntry::setFntFile(const char* value)
{
    if (mLabel)
        mLabel->setFntFile(value);
}

CCPoint PuzzlePageEntry::getMinPoint(void)
{
	if (mLabel)
    {
        // Need to guard against multi-line text returning non-static values.
        std::string str = mLabel->getString();
        mLabel->setString(" "); // All fonts have a space char.
        CCPoint minPt = ccp(mLabel->getPositionX(), mLabel->getPositionY() - mLabel->boundingBox().size.height / 2);
        mLabel->setString(str.c_str());
        return minPt;
    }
	else
		return CCPointZero;
}

CCSize PuzzlePageEntry::getShrunkLabelSizeDiff(void)
{
    float scaleX = mLabel->getScaleX(), scaleY = mLabel->getScaleY();
    if (mIsLabelShrunkToFit && scaleX != 0 && scaleY != 0)
        return CCSizeMake((mLabelBaseScale / scaleX) * mLabel->boundingBox().size.width - mLabel->boundingBox().size.width,
                          (mLabelBaseScale / scaleY) * mLabel->boundingBox().size.height - mLabel->boundingBox().size.height);
    else
        return CCSizeZero;
}

void PuzzlePageEntry::repositionHighlightedIcon(void)
{
    if (mHighlightedIcon && mLabel)
    {
        const char* charStr = getString();
        std::string str = charStr ? charStr : "";
        float langOffsetY = str.length() > 0 ? LangFX::getActiveIconYOffset(str[0]) : LangFX::getActiveIconYOffset();
        langOffsetY += getShrunkLabelSizeDiff().height / 2;
        
        mHighlightedIcon->setPosition(ccp(
                                          mLabel->getPositionX() - (0.5f * mLabel->boundingBox().size.width
                                                                    + mHighlightedIcon->boundingBox().size.width
                                                                    + 36),
                                          mLabel->getPositionY() + mHighlightOffsetY + langOffsetY));
    }
}

void PuzzlePageEntry::localeDidChange(const char* fontKey, const char* FXFontKey)
{
    setString("");
    setFntFile(fontKey);
    setString(" ");
    mHighlightOffsetY = -mLabel->boundingBox().size.height / 2; // Must reset offset because each font has a different base height.
    setString("");
    resizeBoard();
}
