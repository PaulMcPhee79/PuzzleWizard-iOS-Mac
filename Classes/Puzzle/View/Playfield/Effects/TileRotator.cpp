
#include "TileRotator.h"
#include <Prop/CroppedProp.h>
#include <Utils/ReusableCache.h>
#include <Puzzle/View/Playfield/TilePiece.h>
#include <Puzzle/View/Playfield/Effects/TileShadow.h>
#include <Interfaces/ITileDecorator.h>
#include <Utils/FloatTweener.h>
#include <Puzzle/View/Playfield/PlayerPiece.h>
#include <Puzzle/Data/PuzzleTile.h>
#include <Puzzle/View/Playfield/Effects/PuzzleEffects.h>
#include <Utils/Globals.h>
USING_NS_CC;

static const float kRaisedTileScale = 1.6f;
static const float kRaisedShadowScale = 1.8f;
static const float kRaisedAdjacentSeparation = 4.0f;
static const float kTileWidth = 72.0f;
static const float kTileHeight = 72.0f;

static const float kRaiseDurationFactor = 0.45f;
static const float kRotateDurationFactor = 0.45f;
static const float kDropDurationFactor = 0.1f;

static const int kRaiseTagBase = 1000;
static const int kRotateTagBase = 2000;
static const int kDropTagBase = 3000;

static const int kTileZOrder[] = { 1, 2, 0, 3, 4, 7, 5, 6 };

TileRotator::TileRotator(int category)
	:
	Prop(category),
	mHasAnimated(false),
	mState(TileRotator::STATIONARY),
    mEdgeStatus(PuzzleEffects::NONE),
	mRaiseCompletedTag(-1),
	mRotateCompletedTag(-1),
	mDropCompletedTag(-1),
	mPivotPiece(NULL),
	mPivotPlayer(NULL),
	mDecorator(NULL),
	mListener(NULL)
{
    setAnimatableName("TileRotator");
}

TileRotator::~TileRotator(void)
{
	setDecorator(NULL);
	stopAnimating();

	for (int i = 0; i < 2; ++i)
		CC_SAFE_RELEASE_NULL(mLayers[i]);

	for(size_t i = 0; i < mTiles.size(); ++i)
	{
		mTiles[i]->returnToPool();
		CC_SAFE_RELEASE_NULL(mTiles[i]);
	}
    mTiles.clear();

	for(size_t i = 0; i < mTileShadows.size(); ++i)
	{
		CC_SAFE_RELEASE_NULL(mTileShadows[i]);
	}
    mTileShadows.clear();

	if (mPivotPiece)
	{
		mPivotPiece->returnToPool();
		CC_SAFE_RELEASE_NULL(mPivotPiece);
	}

	mPivotPlayer = NULL; // Let owning TilePiece recycle/destroy

	for(size_t i = 0; i < mRaiseTweens.size(); ++i)
	{
		if (mRaiseTweens[i])
			mRaiseTweens[i]->setListener(NULL);
		CC_SAFE_RELEASE_NULL(mRaiseTweens[i]);
	}
    mRaiseTweens.clear();

	for(size_t i = 0; i < mRotateTweens.size(); ++i)
	{
		if (mRotateTweens[i])
			mRotateTweens[i]->setListener(NULL);
		CC_SAFE_RELEASE_NULL(mRotateTweens[i]);
	}
    mRotateTweens.clear();

	for(size_t i = 0; i < mDropTweens.size(); ++i)
	{
		if (mDropTweens[i])
			mDropTweens[i]->setListener(NULL);
		CC_SAFE_RELEASE_NULL(mDropTweens[i]);
	}
    mDropTweens.clear();

	mListener = NULL;
}

TileRotator* TileRotator::createWith(int category, const CCPoint& rotatorOrigin, const CCRect& boardBounds, const std::vector<TilePiece*>& tilePieces, bool autorelease)
{
	TileRotator *rotator = new TileRotator(category);
    if (rotator && rotator->initWith(rotatorOrigin, boardBounds, tilePieces))
    {
		if (autorelease)
			rotator->autorelease();
        return rotator;
    }
    CC_SAFE_DELETE(rotator);
    return NULL;
}

bool TileRotator::initWith(const CCPoint& rotatorOrigin, const CCRect& boardBounds, const std::vector<TilePiece*>& tilePieces)
{
	bool bRet = Prop::init();
	do
	{
		CCAssert(tilePieces.size() >= 8, "TileRotator requires 8 non-null TilePieces.");
		setPosition(rotatorOrigin);
        
        CCNode* shadowLayer = mLayers[0] = new CroppedProp(-1, getShadowOcclusionFromBounds(boardBounds)); // shadow layer
		addChild(shadowLayer);

        CCNode* tileLayer = CCNode::create(); // tile layer
        if (!tileLayer) { bRet = false; break; } else tileLayer->retain();
		mLayers[1] = tileLayer;
        addChild(tileLayer);

		mTiles.resize(tilePieces.size(), NULL);
		mTileShadows.resize(tilePieces.size(), NULL);

        CCPoint origin = ccp(0, 0);
		for (size_t i = 0; i < tilePieces.size() && bRet; ++i)
        {
			int index = kTileZOrder[i];
            CCAssert(tilePieces[index], "TileRotator requires 8 non-null TilePieces.");
			TilePiece* tilePiece = tilePieces[index]->clone();
			if (!tilePiece) { bRet = false; break; } else tilePiece->retain();
			tilePiece->setAestheticState(TilePiece::EDGE);
            
			TileShadow* tileShadow = TileShadow::createWith(mScene->textureByName("tile-shadow"), mShadowOcclusionRegion, false);
			bRet = tileShadow != NULL;
			if (!bRet) break;
			tileShadow->setScale(kTileWidth / tileShadow->boundingBox().size.width);

            switch (index)
            {
                case 0:
                    origin.x = -kTileWidth;
                    origin.y = kTileHeight;
                    break;
                case 1:
                    origin.x = 0;
                    origin.y = kTileHeight;
                    break;
                case 2:
                    origin.x = kTileWidth;
                    origin.y = kTileHeight;
                    break;
                case 3:
                    origin.x = -kTileWidth;
                    origin.y = 0;
                    break;
                case 4:
                    origin.x = kTileWidth;
                    origin.y = 0;
                    break;
                case 5:
                    origin.x = -kTileWidth;
                    origin.y = -kTileHeight;
                    break;
                case 6:
                    origin.x = 0;
                    origin.y = -kTileHeight;
                    break;
                case 7:
                    origin.x = kTileWidth;
                    origin.y = -kTileHeight;
                    break;
            }

			tilePiece->setPosition(origin);
            tileShadow->setPosition(tilePiece->getShadowPosition());
			tileShadow->setPositionY(tileShadow->getPositionY());

			mTiles[index] = tilePiece;
			mTileShadows[index] = tileShadow;
			tileLayer->addChild(tilePiece);
            shadowLayer->addChild(tileShadow);
        }
	} while (false);

    setEdgeStatus(PuzzleEffects::NONE);
	return bRet;
}

float TileRotator::s_shadowOccOverlap = 1.0f;
void TileRotator::setShadowOccOverlap(float value)
{
    s_shadowOccOverlap = value;
}

CCRect TileRotator::getShadowOcclusionFromBounds(const CCRect& bounds)
{
    float overlap = s_shadowOccOverlap;
    return CCRectMake(
                      bounds.origin.x - overlap,
                      bounds.origin.y - overlap,
                      bounds.size.width + 2 * overlap,
                      bounds.size.height + 2 * overlap);
}

void TileRotator::setDecorator(ITileDecorator* value)
{
	if (mDecorator != value)
    {
        mDecorator = value;
        for (std::vector<TilePiece*>::iterator it = mTiles.begin(); it != mTiles.end(); ++it)
			(*it)->setDecorator(value);

        if (mPivotPiece != NULL)
            mPivotPiece->setDecorator(value);
    }
}

void TileRotator::setState(RotatorState value)
{
	if (mState == value)
        return;

    switch (value)
    {
		case TileRotator::STATIONARY:
            break;
        case TileRotator::RAISING:
            mHasAnimated = true;
            break;
        case TileRotator::ROTATING:
            break;
        case TileRotator::DROPPING:
            break;
    }

    mState = value;
}

void TileRotator::setEdgeStatus(int value) {
    if (value == PuzzleEffects::NONE || mEdgeStatus != value) // Only proceed if it changed or it's initializing
    {
        int edgeOverlap = 10;
        CCRect offset = MODE_8x6 ? CCRectMake(-1, -1, 2, 2) : CCRectMake(-1, -1, 1, 1);
        
        if (value & PuzzleEffects::LEFT)
        {
            offset.origin.x -= edgeOverlap;
            offset.size.width += edgeOverlap;
        } else if (value & PuzzleEffects::RIGHT)
        {
            offset.size.width += edgeOverlap;
        }
        
        if (value & PuzzleEffects::TOP)
        {
            offset.size.height += edgeOverlap;
        }
        else if (value & PuzzleEffects::BOTTOM)
        {
            offset.origin.y -= edgeOverlap;
            offset.size.height += edgeOverlap;
        }
        
        mShadowOcclusionRegion = CCRectMake(
                                            offset.origin.x - 1.5f * kTileWidth,
                                            offset.origin.y - 1.5f * kTileHeight,
                                            offset.size.width + 3.0f * kTileWidth,
                                            offset.size.height + 3.0f * kTileHeight);
        
        for (std::vector<TileShadow*>::iterator it = mTileShadows.begin(); it != mTileShadows.end(); ++it)
            (*it)->setOcclusionRegion(mShadowOcclusionRegion);
        
        mEdgeStatus = value;
        
        //CCLog("TileRotator::EdgeStatus = %d", (int)mEdgeStatus);
    }
}

void TileRotator::addPivot(TilePiece* tilePiece, PlayerPiece* playerPiece)
{
	if (tilePiece == NULL || mPivotPiece || mPivotPlayer)
		return;

	mPivotPiece = tilePiece->clone();
	mPivotPiece->retain();
	mPivotPiece->setAestheticState(TilePiece::EDGE);

	CCNode* tileLayer = mLayers[1];
	tileLayer->removeAllChildren();

	for(size_t i = 0; i < mTiles.size(); ++i)
	{
		int index = kTileZOrder[i];
		if (i == 4) // Pivot index
			tileLayer->addChild(mPivotPiece);
		tileLayer->addChild(mTiles[index]);
	}

	if (playerPiece)
	{
		playerPiece->retain();
		mPivotPlayer = playerPiece;
		mPivotPlayer->setPosition(mPivotPiece->boundingBox().size.width / 2, mPivotPiece->boundingBox().size.width / 2 + mPivotPiece->getOffsetY());
		mPivotPiece->addChild(mPivotPlayer);
	}
}

void TileRotator::addPlayerPiece(int index, PlayerPiece* playerPiece)
{
	if (playerPiece == NULL)
        return;

	if ((int)mTiles.size() > index && index >= 0)
	{
		playerPiece->retain();
		playerPiece->setPosition(mTiles[index]->boundingBox().size.width / 2, mTiles[index]->boundingBox().size.width / 2 + mTiles[index]->getOffsetY());
		mTiles[index]->addChild(playerPiece);
	}
	else
	{
		playerPiece->returnToPool();
	}
}

void TileRotator::animate(float duration)
{
	if (mHasAnimated)
        return;
    mHasAnimated = true;
    mDuration = duration;
	setState(TileRotator::RAISING);
    raiseTiles();
}

void TileRotator::raiseTiles(void)
{
	CCAssert(mTiles.size() == mTileShadows.size(), "TileRotator: Bad internal state encountered.");

    float dispersionScale = 0.9f;
	float distX = 0, distY = 0, duration = kRaiseDurationFactor * mDuration;
    float adjacentSep = kRaisedTileScale * kRaisedAdjacentSeparation;
    float raisedScale = kRaisedTileScale, scaleDelta = (kRaisedTileScale - 1.0f);
	for (int i = 0; i < (int)mTiles.size(); ++i)
    {
        TilePiece* tile = mTiles[i];
        TileShadow* shadow = mTileShadows[i];

        switch (i)
        {
            case 0:
				distX = -dispersionScale * raisedScale * kTileWidth;
                distY = scaleDelta * tile->getOffsetY() + scaleDelta * kTileHeight + adjacentSep;
                break;
            case 1:
                distX = 0;
				distY = dispersionScale * raisedScale * (kTileHeight + tile->getOffsetY());
                break;
            case 2:
                distX = scaleDelta * kTileWidth + adjacentSep;
                distY = dispersionScale * raisedScale * (kTileHeight + tile->getOffsetY());
                break;
            case 3:
                distX = -dispersionScale * raisedScale * kTileWidth;
                distY = scaleDelta * tile->getOffsetY();
                break;
            case 4:
                distX = dispersionScale * raisedScale * kTileWidth;
                distY = scaleDelta * tile->getOffsetY();
                break;
            case 5:
                distX = -(adjacentSep + scaleDelta * kTileWidth);
                distY = scaleDelta * tile->getOffsetY() - dispersionScale * raisedScale * kTileHeight;
                break;
            case 6:
                distX = 0;
                distY = scaleDelta * tile->getOffsetY() - dispersionScale * raisedScale * kTileHeight;
                break;
            case 7:
                distX = dispersionScale * raisedScale * kTileWidth;
                distY = scaleDelta * tile->getOffsetY() - (scaleDelta * kTileHeight + adjacentSep);
                break;
        }

		int baseIndex = 6 * i;
		if ((int)mRaiseTweens.size() <= baseIndex)
		{
			for (int j = 0; j < 6; ++j)
			{
				mRaiseTweens.push_back(new FloatTweener(0, this, CMTransitions::EASE_OUT));
				mRaiseTweens[baseIndex + j]->setTag(kRaiseTagBase + baseIndex + j);
				mRaiseCompletedTag = mRaiseTweens[baseIndex + j]->getTag();
			}
		}

		// Tile tweeners
		mRaiseTweens[baseIndex]->reset(tile->getPositionX(), tile->getPositionX() + distX, duration);
		mRaiseTweens[baseIndex+1]->reset(tile->getPositionY(), tile->getPositionY() + distY, duration);
		mRaiseTweens[baseIndex+2]->reset(tile->getScale(), kRaisedTileScale, duration);

		// Shadow tweeners
        CCPoint tilePos = tile->getPosition(), shadowPos = tile->getShadowPosition();
        CCPoint shadowScaledPos = ccp(
                                      shadowPos.x + raisedScale * (shadowPos.x - tilePos.x),
                                      shadowPos.y + raisedScale * (shadowPos.y - tilePos.y));
		mRaiseTweens[baseIndex+3]->reset(shadowPos.x, shadowScaledPos.x + distX, duration);
		mRaiseTweens[baseIndex+4]->reset(shadowPos.y, shadowScaledPos.y + distY, duration);
		mRaiseTweens[baseIndex+5]->reset(shadow->getScale(), kRaisedShadowScale, duration);
    }

	mScene->playSound("rotate");
}

void TileRotator::rotateTiles(void)
{
	float distX = 0, distY = 0, duration = kRotateDurationFactor * mDuration;
    float distFactor = kRaisedTileScale;
	for (int i = 0; i < (int)mTiles.size(); ++i)
    {
        TilePiece* tile = mTiles[i];
        TileShadow* shadow = mTileShadows[i];

        switch (i)
        {
            case 0:
                distX = 0;
                distY = -distFactor * kTileHeight;
                break;
            case 1:
                distX = -distFactor * kTileWidth;
                distY = 0;
                break;
            case 2:
                distX = -distFactor * kTileWidth;
                distY = 0;
                break;
            case 3:
                distX = 0;
                distY = -distFactor * kTileHeight;
                break;
            case 4:
                distX = 0;
                distY = distFactor * kTileHeight;
                break;
            case 5:
                distX = distFactor * kTileWidth;
                distY = 0;
                break;
            case 6:
                distX = distFactor * kTileWidth;
                distY = 0;
                break;
            case 7:
                distX = 0;
                distY = distFactor * kTileHeight;
                break;
        }

		int baseIndex = 4 * i;
		if ((int)mRotateTweens.size() <= baseIndex)
		{
			for (int j = 0; j < 4; ++j)
			{
				mRotateTweens.push_back(new FloatTweener(0, this, CMTransitions::LINEAR));
				mRotateTweens[baseIndex + j]->setTag(kRotateTagBase + baseIndex + j);
				mRotateCompletedTag = mRotateTweens[baseIndex + j]->getTag();
			}
		}

		// Tile tweeners
		mRotateTweens[baseIndex]->reset(tile->getPositionX(), tile->getPositionX() + distX, duration);
		mRotateTweens[baseIndex+1]->reset(tile->getPositionY(), tile->getPositionY() + distY, duration);

		// Shadow tweeners
		mRotateTweens[baseIndex+2]->reset(shadow->getPositionX(), shadow->getPositionX() + distX, duration);
		mRotateTweens[baseIndex+3]->reset(shadow->getPositionY(), shadow->getPositionY() + distY, duration);
    }
}

void TileRotator::dropTiles(void)
{
	float destX = 0, destY = 0, duration = kDropDurationFactor * mDuration;
	for (int i = 0; i < (int)mTiles.size(); ++i)
    {
        TilePiece* tile = mTiles[i];
        TileShadow* shadow = mTileShadows[i];

        switch (i)
        {
			case 0:
				destX = -kTileWidth;
				destY = 0;
				break;
			case 1:
				destX = -kTileWidth;
				destY = kTileHeight;
				break;
			case 2:
				destX = 0;
				destY = kTileHeight;
				break;
			case 3:
				destX = -kTileWidth;
				destY = -kTileHeight;
				break;
			case 4:
				destX = kTileWidth;
				destY = kTileHeight;
				break;
			case 5:
				destX = 0;
				destY = -kTileHeight;
				break;
			case 6:
				destX = kTileWidth;
				destY = -kTileHeight;
				break;
			case 7:
				destX = kTileWidth;
				destY = 0;
				break;
        }

		int baseIndex = 6 * i;
		if ((int)mDropTweens.size() <= baseIndex)
		{
			for (int j = 0; j < 6; ++j)
			{
				mDropTweens.push_back(new FloatTweener(0, this, CMTransitions::EASE_IN));
				mDropTweens[baseIndex + j]->setTag(kDropTagBase + baseIndex + j);
				mDropCompletedTag = mDropTweens[baseIndex + j]->getTag();
			}
		}

		// Tile tweeners
		mDropTweens[baseIndex]->reset(tile->getPositionX(), destX, duration);
		mDropTweens[baseIndex+1]->reset(tile->getPositionY(), destY, duration);
		mDropTweens[baseIndex+2]->reset(tile->getScale(), 1.0f, duration);

		// Shadow tweeners
		mDropTweens[baseIndex+3]->reset(shadow->getPositionX(), destX, duration);
		mDropTweens[baseIndex+4]->reset(shadow->getPositionY(), destY, duration);
		mDropTweens[baseIndex+5]->reset(shadow->getScale(), tile->tileDimensions().width / shadow->boundingBox().size.width, duration);
    }

	//mScene->playSound("rotate-impact");
}

int TileRotator::getBaseIndexFromTag(int tag) const
{
	if (tag >= kDropTagBase)
		return tag - kDropTagBase;
	else if (tag >= kRotateTagBase)
		return tag - kRotateTagBase;
	else if (tag >= kRaiseTagBase)
		return tag - kRaiseTagBase;
	else
	{
		CCAssert(false, "TileRotator::getBaseIndexFromTag invalid tag received.");
		return -1;
	}
}

void TileRotator::onEvent(int evType, void* evData)
{
	if (evType == FloatTweener::EV_TYPE_FLOAT_TWEENER_CHANGED())
	{
		FloatTweener* tweener = static_cast<FloatTweener*>(evData);
		if (tweener)
		{
			float tweenedValue = tweener->getTweenedValue();
			int tag = tweener->getTag();
			int baseIndex = getBaseIndexFromTag(tag);
			
			if (tag >= kDropTagBase || (tag >= kRaiseTagBase && tag < kRotateTagBase))
			{
				int tileIndex = baseIndex / 6;
				int attrIndex = baseIndex - tileIndex * 6;

				TilePiece* tilePiece = mTiles[tileIndex];
				TileShadow* tileShadow = mTileShadows[tileIndex];

				switch (attrIndex)
				{
					case 0: tilePiece->setPositionX(tweenedValue); break;
					case 1: tilePiece->setPositionY(tweenedValue); break;
					case 2: tilePiece->setScale(tweenedValue); break;
					case 3: tileShadow->setPositionX(tweenedValue); break;
					case 4: tileShadow->setPositionY(tweenedValue); break;
					case 5: tileShadow->setScale(tweenedValue); break;
				}
			}
			else if (tag >= kRotateTagBase)
			{
				int tileIndex = baseIndex / 4;
				int attrIndex = baseIndex - tileIndex * 4;

				TilePiece* tilePiece = mTiles[tileIndex];
				TileShadow* tileShadow = mTileShadows[tileIndex];

				switch (attrIndex)
				{
					case 0: tilePiece->setPositionX(tweenedValue); break;
					case 1: tilePiece->setPositionY(tweenedValue); break;
					case 2: tileShadow->setPositionX(tweenedValue); break;
					case 3: tileShadow->setPositionY(tweenedValue); break;
				}
			}
		}
	}
	else if (evType == FloatTweener::EV_TYPE_FLOAT_TWEENER_COMPLETED())
	{
		FloatTweener* tweener = static_cast<FloatTweener*>(evData);
		if (tweener)
		{
			int tag = tweener->getTag();
			if (tag == mRaiseCompletedTag)
			{
				setState(TileRotator::ROTATING);
				rotateTiles();
			}
			else if (tag == mRotateCompletedTag)
			{
				setState(TileRotator::DROPPING);
				dropTiles();
			}
			else if (tag == mDropCompletedTag)
			{
				setState(TileRotator::STATIONARY);
				stopAnimating();

				if (mListener)
					mListener->onEvent(EV_TYPE_ANIMATION_COMPLETED(), this);
			}
		}
	}
}

void TileRotator::stopAnimating(void)
{
	mScene->removeFromJuggler(this);
}

void TileRotator::advanceTime(float dt)
{
	if (!getInUse())
		return;

	// Advance tweeners in reverse state order so we don't skip a frame
	if (getState() == TileRotator::DROPPING)
	{
		for(size_t i = 0; i < mDropTweens.size(); ++i)
			mDropTweens[i]->advanceTime(dt);
	}

	if (getState() == TileRotator::ROTATING)
	{
		for(size_t i = 0; i < mRotateTweens.size(); ++i)
			mRotateTweens[i]->advanceTime(dt);
	}

	if (getState() == TileRotator::RAISING)
	{
		for(size_t i = 0; i < mRaiseTweens.size(); ++i)
			mRaiseTweens[i]->advanceTime(dt);
	}

	if (mPivotPlayer && mPivotPlayer->getInUse())
		mPivotPlayer->advanceTime(dt);
}

/* Reusable Implementation */
ReusableCache* TileRotator::sCache = NULL;
bool TileRotator::sCaching = false;

void TileRotator::setupReusables(void)
{
	if (sCache)
        return;

    sCaching = true;
    sCache = new ReusableCache();

#if CM_SMALL_CACHES
    int cacheSize = 3;
#else
    int cacheSize = 6;
#endif
    uint reuseKey = 0;
    IReusable* reusable = NULL;
    PuzzleTile* tile = new PuzzleTile();
    std::vector<TilePiece*> tilePieces;
    for (int i = 0; i < 8; ++i)
		tilePieces.push_back(TilePiece::getTilePiece(tile));
    tile->clearViews();
    sCache->addKey(cacheSize, reuseKey);

	CCPoint origin = CCPointZero;
	CCRect boardBounds = CCRectMake(0, 0, 1, 1);
    for (int i = 0; i < cacheSize; ++i)
    {
        reusable = getTileRotator(origin, boardBounds, tilePieces);
        reusable->hibernate();
        sCache->addReusable(reusable);
        tile->clearViews();
    }

	for(size_t i = 0; i < tilePieces.size(); ++i)
		tilePieces[i]->returnToPool();

    CC_SAFE_RELEASE_NULL(tile);
    sCache->verifyCacheIntegrity();
    sCaching = false;
}

TileRotator* TileRotator::getTileRotator(const CCPoint& rotatorOrigin, const CCRect& boardBounds, const std::vector<TilePiece*>& tilePieces)
{
	uint reuseKey = 0;
    TileRotator* rotator = static_cast<TileRotator*>(checkoutReusable(reuseKey));

    if (rotator)
    {
        rotator->reuse();
        rotator->reconfigure(rotatorOrigin, boardBounds, tilePieces);
    }
    else
    {
		rotator = TileRotator::createWith(CMGlobals::BOARD, rotatorOrigin, boardBounds, tilePieces, !sCaching);
    }

    return rotator;
}

IReusable* TileRotator::checkoutReusable(uint reuseKey)
{
	IReusable* reusable = NULL;

    if (sCache && !sCaching)
		reusable = sCache->checkout(reuseKey);

    return reusable;
}

void TileRotator::checkinReusable(IReusable* reusable)
{
	if (sCache && !sCaching && reusable)
		sCache->checkin(reusable);
}

void TileRotator::reuse(void)
{
	if (getInUse())
        return;

	mHasAnimated = false;
    setVisible(true);
	setState(TileRotator::STATIONARY);

    mInUse = true;
}

void TileRotator::hibernate(void)
{
	if (!getInUse())
        return;

    stopAnimating();

    for(size_t i = 0; i < mTiles.size(); ++i)
	{
		mTiles[i]->returnToPool();
		CC_SAFE_RELEASE_NULL(mTiles[i]);
	}
    mTiles.clear();

	if (mPivotPiece)
	{
		mPivotPiece->returnToPool();
		CC_SAFE_RELEASE_NULL(mPivotPiece);
	}

	mPivotPlayer = NULL; // Let owning TilePiece recycle/destroy

	setDecorator(NULL);
    setListener(NULL);
	removeFromParent();

    mInUse = false;
    checkinReusable(this);
}

void TileRotator::reconfigure(const CCPoint& rotatorOrigin, const CCRect& boardBounds, const std::vector<TilePiece*>& tilePieces)
{
	CCAssert(tilePieces.size() >= 8, "TileRotator requires 8 non-null TilePieces.");

	setPosition(rotatorOrigin);

    CroppedProp* shadowLayer = static_cast<CroppedProp*>(mLayers[0]);
    shadowLayer->setViewableRegion(getShadowOcclusionFromBounds(boardBounds));

	mTiles.resize(tilePieces.size(), NULL);
    
    CCNode* tileLayer = static_cast<CCNode*>(mLayers[1]);
    CCPoint origin = ccp(0, 0);
    for (size_t i = 0; i < tilePieces.size(); ++i)
    {
		int index = kTileZOrder[i];
        CCAssert(tilePieces[index], "TileRotator requires 8 non-null TilePieces.");
        TilePiece* tilePiece = tilePieces[index]->clone();
		tilePiece->retain();
		tilePiece->setAestheticState(TilePiece::EDGE);

		TileShadow* tileShadow = mTileShadows[index];
		tileShadow->setScale(1);
		tileShadow->setScale(kTileWidth / tileShadow->boundingBox().size.width);

        switch (index)
        {
            case 0:
                origin.x = -kTileWidth;
                origin.y = kTileHeight;
                break;
            case 1:
                origin.x = 0;
                origin.y = kTileHeight;
                break;
            case 2:
                origin.x = kTileWidth;
                origin.y = kTileHeight;
                break;
            case 3:
                origin.x = -kTileWidth;
                origin.y = 0;
                break;
            case 4:
                origin.x = kTileWidth;
                origin.y = 0;
                break;
            case 5:
                origin.x = -kTileWidth;
                origin.y = -kTileHeight;
                break;
            case 6:
                origin.x = 0;
                origin.y = -kTileHeight;
                break;
            case 7:
                origin.x = kTileWidth;
                origin.y = -kTileHeight;
                break;
        }

        tilePiece->setPosition(origin);
		tileShadow->setPosition(origin);

        mTiles[index] = tilePiece;
		tileLayer->addChild(tilePiece);
    }
    
    setEdgeStatus(PuzzleEffects::NONE);
}

void TileRotator::returnToPool(void)
{
    if (getPoolIndex() != -1)
        hibernate();
    else
        removeFromParent();
}
