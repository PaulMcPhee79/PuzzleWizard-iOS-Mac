
#include "TileSwapper.h"
#include <Utils/ReusableCache.h>
#include <Interfaces/IEventListener.h>
#include <Puzzle/View/Playfield/TilePiece.h>
#include <Puzzle/Data/PuzzleTile.h>
#include <Puzzle/View/Playfield/Effects/Dissolver.h>
#include <Utils/CMTypes.h>
#include <Utils/Globals.h>
USING_NS_CC;

static const float kTileWidth = 72.0f;
static const float kTileHeight = 72.0f;

TileSwapper::TileSwapper(int category)
	:
	Prop(category),
	mHasStoppedAnimating(false),
	mThreshold(0.0f),
	mListener(NULL)
{
    setAnimatableName("TileSwapper");
}

TileSwapper::~TileSwapper(void)
{
	for(size_t i = 0; i < mTilePieces.size(); ++i)
	{
		mTilePieces[i]->returnToPool();
		CC_SAFE_RELEASE_NULL(mTilePieces[i]);
	}

	CC_SAFE_RELEASE_NULL(mDissolver);
}

TileSwapper* TileSwapper::createWith(int category, const vec4& swapOrigins, const std::vector<TilePiece*>& tilePieces, bool autorelease)
{
	TileSwapper *tileSwapper = new TileSwapper(category);
    if (tileSwapper && tileSwapper->initWith(swapOrigins, tilePieces))
    {
		if (autorelease)
			tileSwapper->autorelease();
        return tileSwapper;
    }
    CC_SAFE_DELETE(tileSwapper);
    return NULL;
}

bool TileSwapper::initWith(const vec4& swapOrigins, const std::vector<TilePiece*>& tilePieces)
{
	CCAssert(tilePieces.size() > 1 && (tilePieces.size() & 1) == 0, "TileSwapper requires an even number of TilePieces to swap.");
	bool bRet = Prop::init();
	do
	{
		mDissolver = Dissolver::createWith(getCategory(), mScene->textureByName("playfield-atlas"), 1, 0.0f, 1.0f, false);
		if (!mDissolver) { bRet = false; break; }
		mDissolver->setListener(this);
		addChild(mDissolver);

		CCPoint tileOrigin = ccp(0, 0);
		for (int i = 0; i < 2 && bRet; ++i)
		{
			CCPoint swapOrigin = i == 0 ? ccp(swapOrigins.x, swapOrigins.y) : ccp(swapOrigins.z, swapOrigins.w);
			for (int j = 0; j < (int)tilePieces.size() / 2 && bRet; ++j)
			{
				int tileIndex = i * (int)tilePieces.size() / 2 + j;
				if (tilePieces[tileIndex] == NULL)
					continue;

				TilePiece* tilePiece = tilePieces[tileIndex]->clone();
				if (!tilePiece) { bRet = false; break; } else tilePiece->retain();
                if (tilePiece->getData() && !tilePiece->getData()->isEdgeTile())
                    tilePiece->setAestheticState(TilePiece::OCCLUSION);
				tilePiece->setData(NULL);

				switch (j)
				{
					case 0:
						tileOrigin.x = -kTileWidth;
						tileOrigin.y = kTileHeight;
						break;
					case 1:
						tileOrigin.x = 0;
						tileOrigin.y = kTileHeight;
						break;
					case 2:
						tileOrigin.x = kTileWidth;
						tileOrigin.y = kTileHeight;
						break;
					case 3:
						tileOrigin.x = -kTileWidth;
						tileOrigin.y = 0;
						break;
					case 4:
						tileOrigin.x = 0;
						tileOrigin.y = 0;
						break;
					case 5:
						tileOrigin.x = kTileWidth;
						tileOrigin.y = 0;
						break;
					case 6:
						tileOrigin.x = -kTileWidth;
						tileOrigin.y = -kTileHeight;
						break;
					case 7:
						tileOrigin.x = 0;
						tileOrigin.y = -kTileHeight;
						break;
					case 8:
						tileOrigin.x = kTileWidth;
						tileOrigin.y = -kTileHeight;
						break;
				}

				tileOrigin.x += swapOrigin.x;
				tileOrigin.y += swapOrigin.y;

				tilePiece->setPosition(tileOrigin);
				mTilePieces.push_back(tilePiece);
				mDissolver->addDissolvee(tilePiece);
			}
		}
	} while (false);

	return bRet;
}

void TileSwapper::onEvent(int evType, void* evData)
{
	if (evType == Dissolver::EV_TYPE_DISSOLVE_CYCLE_COMPLETED())
	{
		stopAnimating();
		if (mListener)
			mListener->onEvent(EV_TYPE_ANIMATION_COMPLETED(), this);
	}
}

void TileSwapper::advanceTime(float dt)
{
	if (mHasStoppedAnimating)
		return;

	if (mDissolver)
		mDissolver->advanceTime(dt);
}

void TileSwapper::stopAnimating(void)
{
	mHasStoppedAnimating = true;
	mScene->removeFromJuggler(this);
}

/* Reusable Implementation */
ReusableCache* TileSwapper::sCache = NULL;
bool TileSwapper::sCaching = false;

void TileSwapper::setupReusables(void)
{
	if (sCache)
        return;

	sCaching = true;
    sCache = new ReusableCache();

#if CM_SMALL_CACHES
    int cacheSize = 4;
#else
    int cacheSize = 6;
#endif
    uint reuseKey = 0;
    IReusable* reusable = NULL;
    vec4 swapOrigins = CMVec4Zero;
	std::vector<TilePiece*> tilePieces;
    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 9; ++j)
			tilePieces.push_back(TilePiece::getTilePiece(PuzzleTile::create()));
    }
    sCache->addKey(cacheSize, reuseKey);

    for (int i = 0; i < cacheSize; ++i)
    {
        reusable = getTileSwapper(swapOrigins, tilePieces);
        reusable->hibernate();
        sCache->addReusable(reusable);
    }

	sCache->verifyCacheIntegrity();
    sCaching = false;
}

TileSwapper* TileSwapper::getTileSwapper(const vec4& swapOrigins, const std::vector<TilePiece*>& tilePieces)
{
	uint reuseKey = 0;
    TileSwapper* swapper = static_cast<TileSwapper*>(checkoutReusable(reuseKey));

    if (swapper)
    {
        swapper->reuse();
        swapper->reconfigure(swapOrigins, tilePieces);
    }
    else
    {
        swapper = TileSwapper::createWith(CMGlobals::BOARD, swapOrigins, tilePieces, !sCaching);
    }

    return swapper;
}

IReusable* TileSwapper::checkoutReusable(uint reuseKey)
{
	IReusable* reusable = NULL;

    if (sCache && !sCaching)
		reusable = sCache->checkout(reuseKey);

    return reusable;
}

void TileSwapper::checkinReusable(IReusable* reusable)
{
	if (sCache && !sCaching && reusable)
		sCache->checkin(reusable);
}

void TileSwapper::reuse(void)
{
	if (getInUse())
        return;

	mHasStoppedAnimating = false;
    mDissolver->reset();
	setVisible(true);

    mInUse = true;
}

void TileSwapper::hibernate(void)
{
	if (!getInUse())
        return;

	stopAnimating();
	mDissolver->clear();

	for(size_t i = 0; i < mTilePieces.size(); ++i)
	{
		mTilePieces[i]->returnToPool();
		CC_SAFE_RELEASE_NULL(mTilePieces[i]);
	}
	mTilePieces.clear();

	removeFromParent();
	mListener = NULL;

    mInUse = false;
    checkinReusable(this);
}

void TileSwapper::reconfigure(const vec4& swapOrigins, const std::vector<TilePiece*>& tilePieces)
{
	CCPoint tileOrigin = ccp(0, 0);
	for (int i = 0; i < 2; ++i)
	{
		CCPoint swapOrigin = i == 0 ? ccp(swapOrigins.x, swapOrigins.y) : ccp(swapOrigins.z, swapOrigins.w);
		for (int j = 0; j < (int)tilePieces.size() / 2; ++j)
		{
			int tileIndex = i * (int)tilePieces.size() / 2 + j;
			if (tilePieces[tileIndex] == NULL)
				continue;

			TilePiece* tilePiece = tilePieces[tileIndex]->clone();
			tilePiece->retain();
            if (tilePiece->getData() && !tilePiece->getData()->isEdgeTile())
                tilePiece->setAestheticState(TilePiece::OCCLUSION);
			tilePiece->setData(NULL);

			switch (j)
			{
				case 0:
					tileOrigin.x = -kTileWidth;
					tileOrigin.y = kTileHeight;
					break;
				case 1:
					tileOrigin.x = 0;
					tileOrigin.y = kTileHeight;
					break;
				case 2:
					tileOrigin.x = kTileWidth;
					tileOrigin.y = kTileHeight;
					break;
				case 3:
					tileOrigin.x = -kTileWidth;
					tileOrigin.y = 0;
					break;
				case 4:
					tileOrigin.x = 0;
					tileOrigin.y = 0;
					break;
				case 5:
					tileOrigin.x = kTileWidth;
					tileOrigin.y = 0;
					break;
				case 6:
					tileOrigin.x = -kTileWidth;
					tileOrigin.y = -kTileHeight;
					break;
				case 7:
					tileOrigin.x = 0;
					tileOrigin.y = -kTileHeight;
					break;
				case 8:
					tileOrigin.x = kTileWidth;
					tileOrigin.y = -kTileHeight;
					break;
			}

			tileOrigin.x += swapOrigin.x;
			tileOrigin.y += swapOrigin.y;

			tilePiece->setPosition(tileOrigin);
			mTilePieces.push_back(tilePiece);
			mDissolver->addDissolvee(tilePiece);
		}
	}
}

void TileSwapper::returnToPool(void)
{
    if (getPoolIndex() != -1)
        hibernate();
    else
        removeFromParent();
}
