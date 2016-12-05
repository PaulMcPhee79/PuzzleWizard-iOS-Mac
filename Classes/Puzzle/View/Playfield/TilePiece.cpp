
#include "TilePiece.h"
#include <Utils/ReusableCache.h>
#include <Puzzle/Data/PuzzleTile.h>
#include <Puzzle/View/Playfield/TileDecoration.h>
#include <Interfaces/ITileDecorator.h>
#include <Puzzle/View/Playfield/PlayerPiece.h>
#include <Utils/SpriteColorer.h>
#include <Puzzle/View/PuzzleHelper.h>
#include <Utils/Utils.h>
#include <Utils/PWDebug.h>
#include <cmath>
USING_NS_CC;

const float TilePiece::kColorTransitionDuration = 0.5f;
static const float kColorArrowTransitionDuration = 0.25f;
static const float kColorFloodTransitionDuration = 0.25f;
static const float kColorSwirlTransitionDuration = 0.25f;
static const float kPainterDelayDuration = 0.05f;
static const float kColorFillDelayDuration = 0.35f;

static const char* kTileTexNameFull = "tile-full";
static const char* kTileTexNameFullMenu = "tile-full-menu";
static const char* kTileTexNameHalf = "tile-half";
static const char* kTileTexNameHalfMenu = "tile-half-menu";

static const float kOverlapOffsetY = 4;
static const float kEdgeOffsetY = 28;

TilePiece::TilePiece(PuzzleTile* tile)
	:
	mTile(NULL),
	mDecoration(NULL),
	mDecorator(NULL),
	mTileColorer(NULL),
	mAesState(TilePiece::NORMAL),
	mColorMagicActive(false),
    mMenuModeEnabled(false),
	mPathIcon(NULL)
{
	CCAssert(tile, "TilePiece requires non-null tile.");
    PWDebug::tilePieceCount++;
    setTile(tile);
}

TilePiece::~TilePiece(void)
{
	setDecorator(NULL);
	setDecoration(NULL);

	CCArray* children = this->getChildren();
	CCObject* obj;
	CCARRAY_FOREACH(children, obj)
    {
        PlayerPiece* playerPiece = dynamic_cast<PlayerPiece*>(obj);
		if (playerPiece)
		{
			playerPiece->returnToPool();
			CC_SAFE_RELEASE(playerPiece);
		}
    }

	if (mTile)
		mTile->deregisterView(this);
	CC_SAFE_RELEASE_NULL(mTile);

	if (mTileColorer)
	{
		mScene->removeFromJuggler(mTileColorer);
		mTileColorer->setListener(NULL);
	}
	CC_SAFE_RELEASE_NULL(mPathIcon);
	CC_SAFE_RELEASE_NULL(mTileColorer);
}

TilePiece* TilePiece::create(PuzzleTile* tile, bool autorelease)
{
	TilePiece *tilePiece = new TilePiece(tile);
    if (tilePiece && tilePiece->init())
    {
		if (autorelease)
			tilePiece->autorelease();
        return tilePiece;
    }
    CC_SAFE_DELETE(tilePiece);
    return NULL;
}

bool TilePiece::init(void)
{
	bool bRet = CMSprite::init();
	if (bRet)
	{
		setDisplayFrame(mScene->spriteFrameByName("tile-half"));

		mPathIcon = new CCSprite();
		mPathIcon->initWithSpriteFrame(mScene->spriteFrameByName("tile-5"));
		mPathIcon->setVisible(false);
		addChild(mPathIcon);

		mTileColorer = new SpriteColorer(this);
		syncWithData();
	}
	return bRet;
}

CCSize TilePiece::tileDimensions(void) const
{
	return CCSizeMake(72, 72);
}

CCRect TilePiece::tileBounds(void)
{
	CCSize tileDims = tileDimensions();
	CCRect bounds = boundingBox();
	bounds.setRect(bounds.origin.x, bounds.origin.y + getOffsetY(), tileDims.width, tileDims.height);
	return bounds;
}

void TilePiece::setDecorator(ITileDecorator* value)
{
	if (mDecorator != value)
    {
        mDecorator = value;
        if (getDecoration())
            getDecoration()->setDecorator(value);
    }
}

void TilePiece::setDecoration(TileDecoration* value)
{
	if (mDecoration == value)
        return;

	uint newType = 0, prevType = 0;

    if (mDecoration)
    {
		prevType = mDecoration->getType();
        mDecoration->returnToPool();
    }

	CC_SAFE_RETAIN(value);
	CC_SAFE_RELEASE(mDecoration);
    mDecoration = value;

	if (mDecoration)
	{
		mDecoration->setPosition(ccp(
			tileDimensions().width / 2,
			tileDimensions().height / 2 + getOffsetY()));
		mDecoration->setDecorator(getDecorator());
		newType = mDecoration->getType();
        
        if (mMenuModeEnabled)
            mDecoration->enableMenuMode(mMenuModeEnabled);
	}

	if (getDecorator() && (prevType != newType) && (prevType == TilePiece::kTDKKey || newType == TilePiece::kTDKKey))
		getDecorator()->decorationDidChange(this);
}

bool TilePiece::isBatchable(void) const
{
	return getDecoration() == NULL || getDecoration()->getType() != TilePiece::kTDKKey; // PuzzleTile::kTFKey;
}

float TilePiece::getOffsetY(void) const
{
	float offset = 0;
	if (mTile)
	{
		switch (mAesState)
		{
			case TilePiece::NORMAL:
				offset = mTile->isEdgeTile() ? kEdgeOffsetY : kOverlapOffsetY;
				break;
			case TilePiece::EDGE:
				offset = kEdgeOffsetY;
				break;
			case TilePiece::OCCLUSION:
				offset = 0;
				break;
		}
	}
	return offset;
}

CCPoint TilePiece::getShadowPosition(void)
{
	//return this->getPosition();
	return ccp(getPositionX(), getPositionY() - getOffsetY()); //getOffsetY() / 2);
}

void TilePiece::setAestheticState(AestheticState value)
{
	if (mAesState == value)
		return;

	mAesState = value;
	updateEdgeStatus();
}

float TilePiece::getMenuModeTileScale(void) const
{
    return 1.0f;
    
//#ifdef CHEEKY_MOBILE
//    return 1.0f;
//#else
//    float baseScale = MIN(1.0f, MAX(0.1f, mScene->getBaseScale()));
//    float scaleFactor = 0.02f; // baseScale > 0.6f ? 0.01f : (baseScale > 0.425f ? 0.015f : 0.02f);
//    return 1.0f + MIN(0.4f, scaleFactor * (1.0f / (baseScale * baseScale)));
//#endif
}

void TilePiece::updateEdgeStatus(void)
{
	if (mTile == NULL)
		return;

	if ((mTile->isEdgeTile() && mAesState == TilePiece::NORMAL) || mAesState == TilePiece::EDGE)
	{
		setDisplayFrame(mScene->spriteFrameByName(mMenuModeEnabled ? kTileTexNameFullMenu : kTileTexNameFull));
        setScale(mMenuModeEnabled ? getMenuModeTileScale() : 1.0f);
		this->setAnchorPoint(CCPointMake(0.5f, 0.5f + kEdgeOffsetY / boundingBox().size.height / 2));

		if (mPathIcon)
			mPathIcon->setPosition(ccp(mPathIcon->boundingBox().size.width / 2, mPathIcon->boundingBox().size.height / 2 + kEdgeOffsetY));
	}
	else if (!mTile->isEdgeTile() && mAesState == TilePiece::NORMAL)
	{
        setScale(mMenuModeEnabled ? getMenuModeTileScale() : 1.0f);
		setDisplayFrame(mScene->spriteFrameByName(mMenuModeEnabled ? kTileTexNameHalfMenu : kTileTexNameHalf));
		this->setAnchorPoint(CCPointMake(0.5f, 0.5f + kOverlapOffsetY / boundingBox().size.height / 2));

		if (mPathIcon)
			mPathIcon->setPosition(ccp(mPathIcon->boundingBox().size.width / 2, mPathIcon->boundingBox().size.height / 2 + kOverlapOffsetY));
	}
	else if (mAesState == TilePiece::OCCLUSION)
	{
        setScale(1.0f);
		setDisplayFrame(mScene->spriteFrameByName("tile-72"));
		this->setAnchorPoint(CCPointMake(0.5f, 0.5f));

		if (mPathIcon)
			mPathIcon->setPosition(ccp(mPathIcon->boundingBox().size.width / 2, mPathIcon->boundingBox().size.height / 2));
	}

	if (mDecoration)
		mDecoration->setPosition(ccp(
			tileDimensions().width / 2,
			tileDimensions().height / 2 + getOffsetY()));
}

void TilePiece::tilePropertiesDidChange(uint code)
{
	if (code == PuzzleTile::kPropCodeIsEdge)
		updateEdgeStatus();
	else
		syncWithData();
}

void TilePiece::transitionToColor(const ccColor3B& color, float duration, float delay)
{
	mScene->removeFromJuggler(mTileColorer);
	mTileColorer->animateColor(color, duration, delay);
	mScene->addToJuggler(mTileColorer);
}

void TilePiece::updateMenuScale(void)
{
    switch (mAesState)
    {
        case TilePiece::NORMAL:
        case TilePiece::EDGE:
            setScale(mMenuModeEnabled ? getMenuModeTileScale() : 1.0f);
            break;
        case TilePiece::OCCLUSION:
            setScale(1.0f);
            break;
    }
}

void TilePiece::enableMenuMode(bool enable)
{
	if (getDecoration())
		getDecoration()->enableMenuMode(enable);
    
    mMenuModeEnabled = enable;
    updateEdgeStatus();
}

void TilePiece::refreshAesthetics(void)
{
    if (mTile == NULL)
        return;
    
    // Tile color
	uint colorKey = mTile->getColorKey();
	ccColor3B transitionColor = CMUtils::uint2color3B(Puzzles::colorForKey(colorKey));
	transitionToColor(transitionColor);
}

void TilePiece::setPathDisplayFrame(uint colorKey, bool colorMagic)
{
    if (mPathIcon)
    {
        if (colorMagic)
        {
            mPathIcon->setDisplayFrame(mScene->spriteFrameByName("tile-magic"));
        }
        else
        {
            if (colorKey >= kColorKeyRed && colorKey <= kColorKeyWhite)
            {
                if (colorKey == kColorKeyGreen && Puzzles::getColorScheme() == Puzzles::CS_COLOR_BLIND)
                    mPathIcon->setDisplayFrame(mScene->spriteFrameByName(CMUtils::strConcatVal("tile-cb-", colorKey).c_str()));
                else
                    mPathIcon->setDisplayFrame(mScene->spriteFrameByName(CMUtils::strConcatVal("tile-", colorKey).c_str()));
            }
        }
    }
}

void TilePiece::enablePath(bool colorMagic)
{
	if (mPathIcon && mTile)
	{
		mPathIcon->setOpacity(255);
		mPathIcon->setVisible(true);

		uint colorKey = mTile->getColorKey();
        setPathDisplayFrame(colorKey, colorMagic);
		mColorMagicActive = colorMagic;
	}
}

void TilePiece::setPathOpacity(GLubyte value)
{
	if (mPathIcon && mPathIcon->isVisible())
		mPathIcon->setOpacity(value);
}

void TilePiece::disablePath(void)
{
	if (mPathIcon && mPathIcon->isVisible())
	{
		mColorMagicActive = false;
		mPathIcon->setVisible(false);
	}
}

void TilePiece::setTile(PuzzleTile* value)
{
	if (mTile == value)
		return;
	if (mTile)
		mTile->deregisterView(this);
	CC_SAFE_RETAIN(value);
	CC_SAFE_RELEASE(mTile);
    mTile = value;
	if (mTile)
		mTile->registerView(this);
}

void TilePiece::setData(PuzzleTile* tile)
{
	setTile(tile);
    if (mTile)
	{
		updateEdgeStatus();
		syncWithData();
	}
}

void TilePiece::syncWithData(void)
{
	if (mTile == NULL)
        return;

    // Tile color
	uint colorKey = mTile->getColorKey();
	ccColor3B transitionColor = CMUtils::uint2color3B(Puzzles::colorForKey(colorKey));
	switch (mTile->getDecorator())
    {
		case PuzzleTile::kTFColorSwap:
            transitionToColor(transitionColor);
            break;
        case PuzzleTile::kTFPainter:
			transitionToColor(transitionColor, kColorArrowTransitionDuration, mTile->getDecoratorData() * kPainterDelayDuration);
            break;
        case PuzzleTile::kTFColorFill:
            transitionToColor(
                transitionColor,
                1.35f * kColorFloodTransitionDuration,
                sqrtf(4.0f + mTile->getDecoratorData()) / 2.0f * kColorFillDelayDuration - kColorFillDelayDuration);
            break;
        case PuzzleTile::kTFPaintBomb:
            transitionToColor(transitionColor, kColorSwirlTransitionDuration, 2 * mTile->getDecoratorData() * kPainterDelayDuration);
            break;
        default:
			mScene->removeFromJuggler(mTileColorer);
			setColor(transitionColor);
            break;
    }

	// Path Icon
	if (mPathIcon && mPathIcon->isVisible() && !mColorMagicActive && colorKey >= kColorKeyRed && colorKey <= kColorKeyWhite)
        setPathDisplayFrame(colorKey, mColorMagicActive);

    // Tile decoration
	TileDecoration* decoration = Puzzles::decorationForTile(mTile);
	if (decoration && getDecoration())
		decoration->syncWithTileDecoration(getDecoration());

	setDecoration(decoration);

	if (getDecoration())
		addChild(getDecoration());
}

void TilePiece::syncWithTilePiece(TilePiece* tilePiece)
{
	if (tilePiece && getDecoration() && tilePiece->getDecoration())
		getDecoration()->syncWithTileDecoration(tilePiece->getDecoration());
}

void TilePiece::setTeleportOpacity(GLubyte value)
{
	if (mDecoration)
		mDecoration->setTeleportOpacity(value);
}

TilePiece* TilePiece::clone()
{
	TilePiece* tilePiece = TilePiece::getTilePiece(mTile);
	if (tilePiece && mDecoration)
		tilePiece->setTeleportOpacity(mDecoration->getTeleportOpacity());
	return tilePiece;
}

void TilePiece::setColorKeysForSwapKey(uint swapKey, uint& colorKeyLeft, uint& colorKeyRight)
{
	// rb,rg,ry,rw,bg,by,bw,gy,gw,yw
    switch (swapKey >> PuzzleTile::kBitShiftDecorationStyle)
    {
        case 0: colorKeyLeft = kColorKeyRed; colorKeyRight = kColorKeyBlue; break;
        case 1: colorKeyLeft = kColorKeyRed; colorKeyRight = kColorKeyGreen; break;
        case 2: colorKeyLeft = kColorKeyRed; colorKeyRight = kColorKeyYellow; break;
        case 3: colorKeyLeft = kColorKeyRed; colorKeyRight = kColorKeyWhite; break;
        case 4: colorKeyLeft = kColorKeyBlue; colorKeyRight = kColorKeyGreen; break;
        case 5: colorKeyLeft = kColorKeyBlue; colorKeyRight = kColorKeyYellow; break;
        case 6: colorKeyLeft = kColorKeyBlue; colorKeyRight = kColorKeyWhite; break;
        case 7: colorKeyLeft = kColorKeyGreen; colorKeyRight = kColorKeyYellow; break;
        case 8: colorKeyLeft = kColorKeyGreen; colorKeyRight = kColorKeyWhite; break;
        case 9: colorKeyLeft = kColorKeyYellow; colorKeyRight = kColorKeyWhite; break;
        default: colorKeyLeft = kColorKeyWhite; colorKeyRight = kColorKeyWhite; break;
    }
}

/* Reusable Implementation */
ReusableCache* TilePiece::sCache = NULL;
bool TilePiece::sCaching = false;

void TilePiece::setupReusables(void)
{
	if (sCache)
        return;

    sCaching = true;
    sCache = new ReusableCache();

    int cacheSize = 900; // 8 PuzzleBoards (2 transitioners + 6 menus) + some extras for moves.
    uint reuseKey = 0;
    IReusable* reusable = NULL;
    PuzzleTile* tile = new PuzzleTile();
    sCache->addKey(cacheSize, reuseKey);

    for (int i = 0; i < cacheSize; ++i)
    {
        reusable = getTilePiece(tile);
        reusable->hibernate();
        sCache->addReusable(reusable);

		if ((i & 15) == 0)
			tile->clearViews();
    }

    CC_SAFE_RELEASE_NULL(tile);
    sCache->verifyCacheIntegrity();
    sCaching = false;
}

IReusable* TilePiece::checkoutReusable(uint reuseKey)
{
	IReusable* reusable = NULL;

    if (sCache && !sCaching)
		reusable = sCache->checkout(reuseKey);

    return reusable;
}

void TilePiece::checkinReusable(IReusable* reusable)
{
	if (sCache && !sCaching && reusable)
		sCache->checkin(reusable);
}

TilePiece* TilePiece::getTilePiece(PuzzleTile* tile)
{
	uint reuseKey = 0;
    TilePiece* tilePiece = static_cast<TilePiece*>(checkoutReusable(reuseKey));

    if (tilePiece)
    {
        tilePiece->reuse();
        tilePiece->setData(tile);
    }
    else
    {
		tilePiece = TilePiece::create(tile, !sCaching);
    }
    
    return tilePiece;
}

void TilePiece::reuse(void)
{
	if (getInUse())
        return;

	setPosition(ccp(0, 0));
	setOpacityChildren(255);
	setScale(1);
	setRotation(0);
	setVisible(true);
    enableMenuMode(false);
    setAestheticState(TilePiece::NORMAL);
    
    mInUse = true;
}

void TilePiece::hibernate(void)
{
	if (!getInUse())
        return;

    removeFromParent();
    setData(NULL);

	CCArray* children = this->getChildren();
	CCObject* obj = NULL;
	CCARRAY_FOREACH(children, obj)
    {
        PlayerPiece* playerPiece = dynamic_cast<PlayerPiece*>(obj);
		if (playerPiece)
		{
			playerPiece->returnToPool();
			CC_SAFE_RELEASE(playerPiece);
		}
    }

	setDecorator(NULL);
    setDecoration(NULL);
	disablePath();

	if (mTileColorer)
		mScene->removeFromJuggler(mTileColorer);

    mInUse = false;
    checkinReusable(this);
}

void TilePiece::returnToPool(void)
{
    if (getPoolIndex() != -1)
        hibernate();
    else
        removeFromParent();
}
