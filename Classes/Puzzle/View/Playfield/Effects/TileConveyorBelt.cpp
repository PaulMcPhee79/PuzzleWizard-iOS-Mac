
#include "TileConveyorBelt.h"
#include <Utils/ReusableCache.h>
#include <Puzzle/View/Playfield/TilePiece.h>
#include <Puzzle/View/Playfield/PlayerPiece.h>
#include <Puzzle/View/Playfield/Effects/TileShadow.h>
#include <Puzzle/Data/PuzzleTile.h>
#include <Puzzle/View/Playfield/Effects/PuzzleEffects.h>
#include <Prop/CroppedProp.h>
#include <Utils/FloatTweener.h>
#include <Utils/Globals.h>
#include <Utils/Utils.h>
#include <cmath>
USING_NS_CC;

static const float kRaisedTileScale = 1.6f;
static const float kRaisedShadowScale = 1.8f;
static const float kTileWidth = 72.0f;
static const float kTileHeight = 72.0f;

static const float kRaiseDurationFactor = 0.125f;
static const float kSlideDurationFactor = 0.75f;

static const uint kHorizBelt = 1;
static const uint kVertBelt = 2;

TileConveyorBelt::TileConveyorBelt(int category)
	:
Prop(category),
mHasAnimated(false),
mHasStoppedAnimating(false),
mState(TileConveyorBelt::STATIONARY),
mEdgeStatus(PuzzleEffects::NONE),
mWrapIndex(0),
mDuration(0),
mRaiseOffsetY(0),
mShadowBaseScale(1),
mWrapTile(NULL),
mWrapShadowCrop(NULL),
mWrapShadow(NULL),
mMoveBeltTweener(NULL),
mMoveWrapTweener(NULL),
mScaleWrapTweener(NULL),
mRaiseWrapTweener(NULL),
mDecorator(NULL),
mListener(NULL)
{
    setAnimatableName("TileConveyorBelt");
}

TileConveyorBelt::~TileConveyorBelt(void)
{
	setDecorator(NULL);
	stopAnimating();

	for (int i = 0; i < kNumLayers; ++i)
		CC_SAFE_RELEASE_NULL(mLayers[i]);

    mBeltTilePieces.clear(); // Weak references to PuzzleBoard's TilePieces

	for(size_t i = 0; i < mOcclusionPieces.size(); ++i)
	{
		mOcclusionPieces[i]->returnToPool();
		CC_SAFE_RELEASE_NULL(mOcclusionPieces[i]);
	}
    mOcclusionPieces.clear();

	if (mWrapTile)
	{
		mWrapTile->returnToPool();
		CC_SAFE_RELEASE_NULL(mWrapTile);
	}

	CC_SAFE_RELEASE_NULL(mWrapShadow);
    CC_SAFE_RELEASE_NULL(mWrapShadowCrop);

	// TilePiece handles IReusable for PlayerPieces on the wrap tile
	for (std::map<int, PlayerPiece*>::iterator it = mPlayerPieces.begin(); it != mPlayerPieces.end(); ++it)
	{
		PlayerPiece* playerPiece = it->second;
        playerPiece->returnToPool();
		CC_SAFE_RELEASE(playerPiece);
	}
	mPlayerPieces.clear();

	if (mMoveBeltTweener)
	{
		mMoveBeltTweener->setListener(NULL);
		CC_SAFE_RELEASE_NULL(mMoveBeltTweener);
	}

	if (mMoveWrapTweener)
	{
		mMoveWrapTweener->setListener(NULL);
		CC_SAFE_RELEASE_NULL(mMoveWrapTweener);
	}

	if (mScaleWrapTweener)
	{
		mScaleWrapTweener->setListener(NULL);
		CC_SAFE_RELEASE_NULL(mScaleWrapTweener);
	}

	if (mRaiseWrapTweener)
	{
		mRaiseWrapTweener->setListener(NULL);
		CC_SAFE_RELEASE_NULL(mRaiseWrapTweener);
	}
    
    mListener = NULL;
}

TileConveyorBelt* TileConveyorBelt::createWith(int category, const CCRect& boardBounds, Coord animDir, int wrapIndex, std::vector<TilePiece*>& beltTiles, bool autorelease)
{
	TileConveyorBelt *conveyorBelt = new TileConveyorBelt(category);
    if (conveyorBelt && conveyorBelt->initWith(boardBounds, animDir, wrapIndex, beltTiles))
    {
		if (autorelease)
			conveyorBelt->autorelease();
        return conveyorBelt;
    }
    CC_SAFE_DELETE(conveyorBelt);
    return NULL;
}

bool TileConveyorBelt::initWith(const CCRect& boardBounds, Coord animDir, int wrapIndex, std::vector<TilePiece*>& beltTiles)
{
	CCAssert(abs(animDir.x + animDir.y) == 1, "Invalid TileConveyorBelt animation direction");

	bool bRet = Prop::init();
	do
	{
		mAnimDir = animDir;
		mWrapIndex = wrapIndex;

		CCNode* occLayer = mLayers[kOccLayerIndex] = new CCNode();
		CCNode* playerLayer = mLayers[kPlayerLayerIndex] = new CCNode();
		CCNode* wrapLayer = mLayers[kWrapLayerIndex] = new CCNode();

		addChild(occLayer);
		addChild(playerLayer);
		addChild(wrapLayer);

		// Belt
		for (size_t i = 0; i < beltTiles.size() && bRet; ++i)
		{
			TilePiece* tilePiece = beltTiles[i];
			tilePiece->setVisible(i != wrapIndex);
			mBeltTilePieces.push_back(tilePiece);

			if (animDir.y == 1 && i == wrapIndex - 1)
				tilePiece->setAestheticState(TilePiece::EDGE);
		}

		if (!bRet) break;

		// Invisible occlusion helpers
		int occFactor = wrapIndex == 0 ? (int)beltTiles.size() : -(int)beltTiles.size();
		for (int i = 0; i < 2; ++i)
		{
			TilePiece* tilePiece = beltTiles[wrapIndex]->clone();
			if (!tilePiece) { bRet = false; break; } else tilePiece->retain();
			tilePiece->setAestheticState(TilePiece::OCCLUSION);
			tilePiece->setPosition(beltTiles[wrapIndex]->getPosition());
			tilePiece->setVisible(false);
        
			if (i == 1)
			{
				if (animDir.x != 0)
					tilePiece->setPositionX(tilePiece->getPositionX() + occFactor * kTileWidth);
				else
					tilePiece->setPositionY(tilePiece->getPositionY() - occFactor * kTileHeight);
			}

			mOcclusionPieces.push_back(tilePiece);
			occLayer->addChild(tilePiece);
		}

		if (!bRet) break;

		// Wrap
		mWrapTile = beltTiles[wrapIndex]->clone();
		if (!mWrapTile) { bRet = false; break; } else mWrapTile->retain();
		mWrapTile->setAestheticState(TilePiece::EDGE);
		mWrapTile->setPosition(beltTiles[wrapIndex]->getPosition());
        
        mRaiseOffsetY = 1.325f * (kRaisedTileScale - 1.0f) * (kTileHeight + mWrapTile->getOffsetY());

		mWrapShadow = TileShadow::createWith(mScene->textureByName("tile-shadow"), mOcclusionPieces[0]->boundingBox(), false);
		bRet = mWrapShadow != NULL;
		if (!bRet) break;
		mWrapShadow->setPosition(mWrapTile->getShadowPosition());
        mShadowBaseScale = kTileWidth / mWrapShadow->boundingBox().size.width;
		mWrapShadow->setScale(mShadowBaseScale);

        mWrapShadowCrop = new CroppedProp(-1, boardBounds);
        mWrapShadowCrop->addChild(mWrapShadow);
        
		wrapLayer->addChild(mWrapShadowCrop);
		wrapLayer->addChild(mWrapTile);

		// Tweeners
		mMoveBeltTweener = new FloatTweener(0, this, CMTransitions::LINEAR);
		mMoveWrapTweener = new FloatTweener(0, this, CMTransitions::LINEAR);
		mScaleWrapTweener = new FloatTweener(0, this, CMTransitions::EASE_OUT);
		mRaiseWrapTweener = new FloatTweener(0, this, CMTransitions::EASE_OUT);
		bRet = mMoveBeltTweener && mMoveWrapTweener && mScaleWrapTweener && mRaiseWrapTweener;
	} while (false);

	return bRet;
}

uint TileConveyorBelt::getOrientation(void) const
{
	return mAnimDir.x != 0 ? kHorizBelt : kVertBelt;
}

void TileConveyorBelt::setDecorator(ITileDecorator* value)
{
	if (mDecorator != value)
    {
        mDecorator = value;
        if (mWrapTile != NULL)
            mWrapTile->setDecorator(value);
    }
}
	
void TileConveyorBelt::setState(CBeltState value)
{
	if (mState == value)
        return;

    switch (value)
    {
		case TileConveyorBelt::STATIONARY:
            break;
		case TileConveyorBelt::RAISING:
            mHasAnimated = true;
            break;
		case TileConveyorBelt::SLIDING:
            break;
		case TileConveyorBelt::DROPPING:
            break;
    }

    mState = value;
}

float TileConveyorBelt::getScaleWrapProxy(void)
{
	return mWrapTile->getScale();
}

void TileConveyorBelt::setScaleWrapProxy(float value)
{
	mWrapTile->setScale(value);
	float shadowScale = mShadowBaseScale + ((value - 1.0f) / (kRaisedTileScale - 1.0f)) * (kRaisedShadowScale - mShadowBaseScale);
	mWrapShadow->setScale(shadowScale);
}

float TileConveyorBelt::getMoveWrapProxy(void)
{
	return getOrientation() == kHorizBelt ? mWrapTile->getPositionX() : mWrapTile->getPositionY();
}

void TileConveyorBelt::syncWrapShadowPosition(void)
{
    if (mWrapTile && mWrapShadow && kRaisedShadowScale > mShadowBaseScale)
    {
        float raisePercent = (mWrapShadow->getScaleY() - mShadowBaseScale) / (kRaisedShadowScale - mShadowBaseScale);
        CCPoint tilePos = mWrapTile->getPosition(), shadowPos = mWrapTile->getShadowPosition();
        mWrapShadow->setPositionY(shadowPos.y + raisePercent * kRaisedShadowScale * (shadowPos.y - tilePos.y));
        
        // Even though this commented section is accurate, it doesn't look as smooth when viewed at normal speed.
//        mWrapShadow->setPositionY(tilePos.y +
//                                  raisePercent * (shadowPos.y - tilePos.y) +
//                                  raisePercent * kRaisedShadowScale * (shadowPos.y - tilePos.y));
    }
}

void TileConveyorBelt::setMoveWrapProxy(float value)
{
	if (getOrientation() == kHorizBelt)
    {
		mWrapTile->setPositionX(value);
		mWrapShadow->setPositionX(value);
    }
    else
    {
        mWrapTile->setPositionY(value);
        syncWrapShadowPosition();
    }
}

float TileConveyorBelt::getRaiseWrapProxy(void)
{
	return mWrapTile ? mWrapTile->getPositionY() : 0;
}

void TileConveyorBelt::setRaiseWrapProxy(float value)
{
	if (mWrapTile)
    {
		mWrapTile->setPositionY(value);
        syncWrapShadowPosition();
    }
}

float TileConveyorBelt::getMoveBeltProxy(void)
{
	return mBeltMovement;
}

void TileConveyorBelt::setMoveBeltProxy(float value)
{
	float delta = value - mBeltMovement;
    mBeltMovement = value;

    if (getOrientation() == kHorizBelt)
    {
		for (size_t i = 0; i < mBeltTilePieces.size(); ++i)
			mBeltTilePieces[i]->setPositionX(mBeltTilePieces[i]->getPositionX() + delta);
		for (size_t i = 0; i < mOcclusionPieces.size(); ++i)
			mOcclusionPieces[i]->setPositionX(mOcclusionPieces[i]->getPositionX() + delta);
    }
    else
    {
		for (size_t i = 0; i < mBeltTilePieces.size(); ++i)
			mBeltTilePieces[i]->setPositionY(mBeltTilePieces[i]->getPositionY() + delta);
		for (size_t i = 0; i < mOcclusionPieces.size(); ++i)
			mOcclusionPieces[i]->setPositionY(mOcclusionPieces[i]->getPositionY() + delta);
    }

	for (std::map<int, PlayerPiece*>::iterator it = mPlayerPieces.begin(); it != mPlayerPieces.end(); ++it)
		it->second->setPosition(mBeltTilePieces[it->first]->getPosition());
}

void TileConveyorBelt::addPlayerPiece(int index, PlayerPiece* playerPiece)
{
	if (playerPiece == NULL)
        return;

    if (index == mWrapIndex)
	{
		playerPiece->retain();
		playerPiece->setPosition(mWrapTile->boundingBox().size.width / 2, mWrapTile->boundingBox().size.height / 2 + mWrapTile->getOffsetY());
		mWrapTile->addChild(playerPiece);
	}
	else if ((int)mBeltTilePieces.size() > index && index >= 0 && mPlayerPieces.count(index) == 0)
	{
		playerPiece->retain();
		playerPiece->setPosition(mBeltTilePieces[index]->getPosition());
		mLayers[kPlayerLayerIndex]->addChild(playerPiece);
		mPlayerPieces[index] = playerPiece;
	}
	else
	{
        playerPiece->returnToPool();
	}
}

void TileConveyorBelt::animate(float duration)
{
	if (mHasAnimated)
        return;
    mDuration = duration;
	setState(TileConveyorBelt::RAISING);
    raiseWrapTile();
}

void TileConveyorBelt::stopAnimating(void)
{
	mHasStoppedAnimating = true;
	mScene->removeFromJuggler(this);
}

void TileConveyorBelt::raiseWrapTile(void)
{
	mScaleWrapTweener->setTransition(CMTransitions::EASE_OUT);
	mScaleWrapTweener->reset(getScaleWrapProxy(), kRaisedTileScale, kRaiseDurationFactor * mDuration);

	mRaiseWrapTweener->setTransition(CMTransitions::EASE_OUT);
	mRaiseWrapTweener->reset(getRaiseWrapProxy(), getRaiseWrapProxy() + mRaiseOffsetY, kRaiseDurationFactor * mDuration);
}

void TileConveyorBelt::slideWrapTile(void)
{
	float dist = getOrientation() == kHorizBelt
		? -mAnimDir.x * ((int)mBeltTilePieces.size() - 1) * kTileWidth
        : mAnimDir.y * ((int)mBeltTilePieces.size() - 1) * kTileHeight;
    float duration = kSlideDurationFactor * mDuration;
    mMoveWrapTweener->reset(getMoveWrapProxy(), getMoveWrapProxy() + dist, duration);
    if (MODE_8x6)
        mScene->playSound(getOrientation() == kHorizBelt ? "cbelt-horiz_8x6" : "cbelt-vert_8x6");
    else
        mScene->playSound(getOrientation() == kHorizBelt ? "cbelt-horiz" : "cbelt-vert");
}

void TileConveyorBelt::slideBeltTiles(void)
{
    float dist = getOrientation() == kHorizBelt
        ? mAnimDir.x * kTileWidth
        : -mAnimDir.y * kTileHeight;

    mBeltMovement = 0;
    mMoveBeltTweener->reset(mBeltMovement, dist, kSlideDurationFactor * mDuration);
}

void TileConveyorBelt::dropWrapTile(void)
{
	mScaleWrapTweener->setTransition(CMTransitions::EASE_IN);
	mScaleWrapTweener->reset(getScaleWrapProxy(), 1.0f, kRaiseDurationFactor * mDuration);

	mRaiseWrapTweener->setTransition(CMTransitions::EASE_IN);
	mRaiseWrapTweener->reset(getRaiseWrapProxy(), getRaiseWrapProxy() - mRaiseOffsetY, kRaiseDurationFactor * mDuration);

	//mScene->playSound("rotate-impact");
}

void TileConveyorBelt::onEvent(int evType, void* evData)
{
	if (evType == FloatTweener::EV_TYPE_FLOAT_TWEENER_CHANGED())
	{
		FloatTweener* tweener = static_cast<FloatTweener*>(evData);
		if (tweener)
		{
			float tweenedValue = tweener->getTweenedValue();
			if (tweener == mMoveBeltTweener)
				setMoveBeltProxy(tweenedValue);
			else if (tweener == mMoveWrapTweener)
				setMoveWrapProxy(tweenedValue);
			else if (tweener == mScaleWrapTweener)
				setScaleWrapProxy(tweenedValue);
			else if (tweener == mRaiseWrapTweener)
				setRaiseWrapProxy(tweenedValue);
		}
	}
	else if (evType == FloatTweener::EV_TYPE_FLOAT_TWEENER_COMPLETED())
	{
		FloatTweener* tweener = static_cast<FloatTweener*>(evData);
		if (tweener)
		{
			switch (getState())
			{
				case TileConveyorBelt::RAISING:
					if (tweener == mScaleWrapTweener)
					{
						setState(TileConveyorBelt::SLIDING);
						slideWrapTile();
						slideBeltTiles();
					}
					break;
				case TileConveyorBelt::SLIDING:
					if (tweener == mMoveWrapTweener)
					{
						setState(TileConveyorBelt::DROPPING);
						dropWrapTile();
					}
					break;
				case TileConveyorBelt::DROPPING:
					if (tweener == mScaleWrapTweener)
						setState(TileConveyorBelt::STATIONARY);
					break;
				default:
					break;
			}
		}
	}
}

void TileConveyorBelt::advanceTime(float dt)
{
	if (!getInUse())
		return;
    
    CBeltState state = getState();
    
	// Advance tweeners in reverse state order so we don't skip a frame. Don't combine RAISING and DROPPING.
	if (getState() == TileConveyorBelt::DROPPING)
	{
		mRaiseWrapTweener->advanceTime(dt);
		mScaleWrapTweener->advanceTime(dt);
	}

	if (getState() == TileConveyorBelt::SLIDING)
	{
		mMoveBeltTweener->advanceTime(dt);
		mMoveWrapTweener->advanceTime(dt);
	}

	if (getState() == TileConveyorBelt::RAISING)
	{
		mRaiseWrapTweener->advanceTime(dt);
		mScaleWrapTweener->advanceTime(dt);
	}

	for (std::map<int, PlayerPiece*>::iterator it = mPlayerPieces.begin(); it != mPlayerPieces.end(); ++it)
	{
		PlayerPiece* playerPiece = it->second;
		if (playerPiece && playerPiece->getInUse())
			playerPiece->advanceTime(dt);
	}
    
    if (state != TileConveyorBelt::STATIONARY && getState() == TileConveyorBelt::STATIONARY)
    {
        stopAnimating();
        
        if (mListener)
            mListener->onEvent(EV_TYPE_ANIMATION_COMPLETED(), this);
    }
}

// Defines padded bounds for use in erasing occlusion edge artifacts due to scaling. Bounds are
// defined so as to only pad towards the edge of the board.
CCRect TileConveyorBelt::getOcclusionOverlap(int occIndex) // occIndex: 0 => wrap tile, 1 => far end tile
{
    int edgeStatus = PuzzleEffects::NONE;
    int edgeOverlap = 10;
    uint orientation = getOrientation();
    
    if (orientation == kHorizBelt)
    {
        if (mWrapIndex == 0)
            edgeStatus |= occIndex == 0 ? PuzzleEffects::LEFT : PuzzleEffects::RIGHT; // Left-moving belt (ie right-moving wrap tile)
        else
            edgeStatus |= occIndex == 0 ? PuzzleEffects::RIGHT : PuzzleEffects::LEFT; // Right-moving belt
        edgeStatus |= mEdgeStatus & (PuzzleEffects::TOP | PuzzleEffects::BOTTOM);
    }
    else // kVertBelt
    {
        if (mWrapIndex == 0)
            edgeStatus |= occIndex == 0 ? PuzzleEffects::TOP : PuzzleEffects::BOTTOM; // Up-moving belt
        else
            edgeStatus |= occIndex == 0 ? PuzzleEffects::BOTTOM : PuzzleEffects::TOP; // Down-moving belt
        edgeStatus |= mEdgeStatus & (PuzzleEffects::LEFT | PuzzleEffects::RIGHT);
    }
    
    CCRect occOverlap = CCRectZero;
    if (edgeStatus & PuzzleEffects::LEFT)
    {
        occOverlap.setRect(
                           occOverlap.origin.x - edgeOverlap,
                           occOverlap.origin.y,
                           occOverlap.size.width + edgeOverlap,
                           occOverlap.size.height);
        //CCLog("PADDING LEFT");
        
    }
    else if (edgeStatus & PuzzleEffects::RIGHT)
    {
        occOverlap.setRect(
                           occOverlap.origin.x,
                           occOverlap.origin.y,
                           occOverlap.size.width + edgeOverlap,
                           occOverlap.size.height);
        //CCLog("PADDING RIGHT");
    }
    
    if (edgeStatus & PuzzleEffects::TOP)
    {
        occOverlap.setRect(
                           occOverlap.origin.x,
                           occOverlap.origin.y,
                           occOverlap.size.width,
                           occOverlap.size.height + edgeOverlap);
        //CCLog("PADDING TOP");
    }
    else if (edgeStatus & PuzzleEffects::BOTTOM)
    {
        occOverlap.setRect(
                           occOverlap.origin.x,
                           occOverlap.origin.y - edgeOverlap,
                           occOverlap.size.width,
                           occOverlap.size.height + edgeOverlap);
        //CCLog("PADDING BOTTOM");
    }
    
    return occOverlap;
}

void TileConveyorBelt::visit(void)
{
	if (mWrapShadow)
	{
		CCRect shadowBounds = mWrapShadow->getShadowBounds();
		for (int i = 0; i < (int)mOcclusionPieces.size(); ++i)
        {
			CCRect occBounds = mOcclusionPieces[i]->boundingBox();
			CCRect occRegion = CMUtils::intersectionRect(shadowBounds, occBounds);

			if (occRegion.size.width > 0 && occRegion.size.height > 0)
			{
                CCRect overlap = getOcclusionOverlap(i);
                occBounds.setRect(
                                  occBounds.origin.x + overlap.origin.x,
                                  occBounds.origin.y + overlap.origin.y,
                                  occBounds.size.width + overlap.size.width,
                                  occBounds.size.height + overlap.size.height);
                mWrapShadow->setOcclusionRegion(occBounds);
				break;
			}
        }
	}

	Prop::visit();
}

/* Reusable Implementation */
ReusableCache* TileConveyorBelt::sCache = NULL;
bool TileConveyorBelt::sCaching = false;

void TileConveyorBelt::setupReusables(void)
{
	if (sCache)
        return;

    uint reuseKeys[] = { kHorizBelt, kVertBelt };
    sCaching = true;
    sCache = new ReusableCache();

#if CM_SMALL_CACHES
    int cacheSize = 3;
#else
    int cacheSize = 6;
#endif
    IReusable* reusable = NULL;
    PuzzleTile* tile = new PuzzleTile();
	std::vector<TilePiece*> horizBeltTiles;
    
    int iMax = MODE_8x6 ? 8 : 10;
	for (int i = 0; i < iMax; ++i)
		horizBeltTiles.push_back(TilePiece::getTilePiece(tile));
	tile->clearViews();

	std::vector<TilePiece*> vertBeltTiles;
    iMax = MODE_8x6 ? 6 : 8;
    for (int i = 0; i < iMax; ++i)
		vertBeltTiles.push_back(TilePiece::getTilePiece(tile));
	tile->clearViews();

	CCRect boardBounds = CCRectMake(0, 0, 1, 1);
    for (int i = 0; i < 2; ++i)
    {
        sCache->addKey(cacheSize, reuseKeys[i]);

        Coord animDir = i == 0 ? cmc(1, 0) : cmc(0, 1);
        std::vector<TilePiece*>* beltTiles = i == 0 ? &horizBeltTiles : &vertBeltTiles;
        for (int j = 0; j < cacheSize; ++j)
        {
            reusable = getTileConveyorBelt(boardBounds, animDir, 0, *beltTiles);
            reusable->hibernate();
            sCache->addReusable(reusable);
			tile->clearViews();
        }
    }

    CC_SAFE_RELEASE_NULL(tile);
    sCache->verifyCacheIntegrity();
    sCaching = false;
}

TileConveyorBelt* TileConveyorBelt::getTileConveyorBelt(const CCRect& boardBounds, Coord animDir, int wrapIndex, std::vector<TilePiece*>& beltTiles)
{
	uint reuseKey = (uint)(animDir.x != 0 ? kHorizBelt : kVertBelt);
    TileConveyorBelt* conveyorBelt = static_cast<TileConveyorBelt*>(checkoutReusable(reuseKey));

    if (conveyorBelt)
    {
        conveyorBelt->reuse();
        conveyorBelt->reconfigure(boardBounds, animDir, wrapIndex, beltTiles);
    }
    else
    {
		conveyorBelt = TileConveyorBelt::createWith(CMGlobals::BOARD, boardBounds, animDir, wrapIndex, beltTiles, !sCaching);
    }

    return conveyorBelt;
}

IReusable* TileConveyorBelt::checkoutReusable(uint reuseKey)
{
	IReusable* reusable = NULL;

    if (sCache && !sCaching)
		reusable = sCache->checkout(reuseKey);

    return reusable;
}

void TileConveyorBelt::checkinReusable(IReusable* reusable)
{
	if (sCache && !sCaching && reusable)
		sCache->checkin(reusable);
}

void TileConveyorBelt::reuse(void)
{
	if (getInUse())
        return;

    mHasAnimated = false;
    mHasStoppedAnimating = false;
    setVisible(true);
    mBeltMovement = 0;
    mMoveBeltTweener->reset(0);
    mMoveWrapTweener->reset(0);
    mScaleWrapTweener->reset(0);
	mRaiseWrapTweener->reset(0);
	setState(TileConveyorBelt::STATIONARY);

    mInUse = true;
}

void TileConveyorBelt::hibernate(void)
{
	if (!getInUse())
        return;

	stopAnimating();

	mLayers[kOccLayerIndex]->removeAllChildren();
	mLayers[kPlayerLayerIndex]->removeAllChildren();

	mBeltTilePieces.clear(); // Weak references to PuzzleBoard's TilePieces

	for (size_t i = 0; i < mOcclusionPieces.size(); ++i)
    {
        mOcclusionPieces[i]->returnToPool();
        CC_SAFE_RELEASE_NULL(mOcclusionPieces[i]);
    }
	mOcclusionPieces.clear();

    mWrapTile->returnToPool();
    CC_SAFE_RELEASE_NULL(mWrapTile);

	// TilePiece handles IReusable for PlayerPieces on the wrap tile
	for (std::map<int, PlayerPiece*>::iterator it = mPlayerPieces.begin(); it != mPlayerPieces.end(); ++it)
	{
		PlayerPiece* playerPiece = it->second;
		playerPiece->returnToPool();
		CC_SAFE_RELEASE(playerPiece);
	}
	mPlayerPieces.clear();

	setDecorator(NULL);
    mListener = NULL;
	removeFromParent();

    mInUse = false;
    checkinReusable(this);
}

void TileConveyorBelt::reconfigure(const CCRect& boardBounds, Coord animDir, int wrapIndex, std::vector<TilePiece*>& beltTiles)
{
	CCAssert(abs(animDir.x + mAnimDir.x) != 1 && abs(animDir.y + mAnimDir.y) != 1, "Invalid TileConveyorBelt::reconfigure animation direction");

    mAnimDir = animDir;
    mWrapIndex = wrapIndex;
    mWrapShadowCrop->setViewableRegion(boardBounds);

    CCNode* occLayer = mLayers[kOccLayerIndex];
    CCNode* wrapLayer = mLayers[kWrapLayerIndex];

    // Belt
	for (int i = 0; i < (int)beltTiles.size(); ++i)
    {
		TilePiece* tilePiece = beltTiles[i];
		tilePiece->setVisible(i != wrapIndex);
		mBeltTilePieces.push_back(tilePiece);

		if (animDir.y == 1 && i == wrapIndex - 1)
			tilePiece->setAestheticState(TilePiece::EDGE);
    }

    // Invisible occlusion helpers
	int occFactor = wrapIndex == 0 ? (int)beltTiles.size() : -(int)beltTiles.size();
	for (int i = 0; i < 2; ++i)
	{
		TilePiece* tilePiece = beltTiles[wrapIndex]->clone();
		tilePiece->retain();
		tilePiece->setAestheticState(TilePiece::OCCLUSION);
		tilePiece->setPosition(beltTiles[wrapIndex]->getPosition());
		tilePiece->setVisible(false);
    
		if (i == 1)
		{
			if (animDir.x != 0)
				tilePiece->setPositionX(tilePiece->getPositionX() + occFactor * kTileWidth);
			else
				tilePiece->setPositionY(tilePiece->getPositionY() - occFactor * kTileHeight);
		}

		mOcclusionPieces.push_back(tilePiece);
		occLayer->addChild(tilePiece);
	}

    // Wrap
	mWrapTile = beltTiles[wrapIndex]->clone();
	mWrapTile->retain();
	mWrapTile->setScale(1.0f);
	mWrapTile->setAestheticState(TilePiece::EDGE);
	mWrapTile->setPosition(beltTiles[wrapIndex]->getPosition());
	wrapLayer->addChild(mWrapTile);

	// Shadow
	mWrapShadow->setPosition(mWrapTile->getShadowPosition());
	mWrapShadow->setOcclusionRegion(mOcclusionPieces[0]->boundingBox());
	mWrapShadow->setScale(1.0f);
	mWrapShadow->setScale(kTileWidth / mWrapShadow->boundingBox().size.width);
    
    setEdgeStatus(PuzzleEffects::NONE);
}

void TileConveyorBelt::returnToPool(void)
{
    if (getPoolIndex() != -1)
        hibernate();
    else
        removeFromParent();
}
