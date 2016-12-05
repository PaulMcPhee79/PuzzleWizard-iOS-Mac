
#include "PuzzleBoard.h"
#include <Puzzle/View/Playfield/TilePiece.h>
#include <Puzzle/View/Playfield/TileDecoration.h>
#include <Puzzle/View/Playfield/PlayerPiece.h>
#include <Puzzle/View/PuzzleHelper.h>
#include <Puzzle/Controllers/ShieldManager.h>
#include <Puzzle/Data/Player.h>
#include <Puzzle/Data/MirroredPlayer.h>
#include <Utils/ByteTweener.h>
#include <Utils/FloatTweener.h>
#include <Extensions/MovieClip.h>
#include <Puzzle/View/Playfield/Effects/TileRotator.h>
#include <Puzzle/View/Playfield/Effects/TileConveyorBelt.h>
#include <Puzzle/View/Playfield/Effects/TileSwapper.h>
#include <Puzzle/Controllers/PuzzleController.h>
#include <SceneControllers/PlayfieldController.h>
#include <Puzzle/View/Playfield/Effects/PuzzleEffects.h>
#include <Utils/Utils.h>
#include <Utils/Globals.h>
#include <Utils/PWDebug.h>
#include <algorithm>
USING_NS_CC;

// Don't overlap transition in/out tags. They require space equal to the board's tile count.
static const int kTransitionInTag = 1000;
static const int kTransitionOutTag = 2000;

// Don't overlap these tags with the transition tags
static const int kByteTweenerPathTag = 3001;
static const int kByteTweenerTeleportTag = 3002;
static const int kByteTweenerKeyTag = 3003;
static const int kByteTweenerIqTag = 3004;

static const float kPathNotFoundDuration = 1.0f;
static const float kTileScaleX = 88.0f / 72.0f;

static const int kIQLabelIndexSML = 0;
static const int kIQLabelIndexLGE = 1;

PuzzleBoard::PuzzleBoard(int category)
	:
	Prop(category),
	mIsLocked(false),
    mIsBatchingEnabled(true),
	mIsTransitioning(false),
    mIsMenuModeEnabled(false),
	mLastPlayerTransitionDelay(0),
	mPuzzle(NULL),
	mPlayerPieces(NULL),
	mNumPathIndexes(0),
	mAddQueue(NULL),
	mRemoveQueue(NULL),
	mPathIndexes(NULL),
	mPathTweener(NULL),
    mPathNotFoundTimer(0),
    mPathNotFound(NULL),
	mTeleportTweener(NULL),
	mKeyTweener(NULL),
    mColorArrowClip(NULL),
    mIqLabel(),
    mIqTag(NULL),
    mIqTagNode(NULL),
    mIqTweener(NULL),
	mHighlighter(NULL),
	mPreBatch(NULL),
	mPostBatch(NULL),
	mKeyNode(NULL),
	mShieldManager(NULL)
{
    PWDebug::puzzleBoardCount++;
    setAnimatableName("PuzzleBoard");
}

PuzzleBoard::~PuzzleBoard(void)
{
    mScene->deregisterResDependent(this);
    mScene->deregisterLocalizable(this);
    forceCompleteActiveAnimations();
    
    enableBatching(true);
    unbatchAllTilePieces();
    
	for(std::vector<TilePiece*>::iterator it = mTilePieces.begin(); it != mTilePieces.end(); ++it)
	{
		TilePiece* tilePiece = *it;
		if (tilePiece)
            tilePiece->returnToPool();
		CC_SAFE_RELEASE(tilePiece);
	}
	mTilePieces.clear();

	for (int i = 0; i < (int)mTransitions.size(); ++i)
	{
		mTransitions[i]->setListener(NULL);
		CC_SAFE_RELEASE_NULL(mTransitions[i]);
	}
	mTransitions.clear();

    if (mPlayerPieces)
    {
		CCDictElement* kvp;
		CCDICT_FOREACH(mPlayerPieces, kvp)
		{
			PlayerPiece* playerPiece = static_cast<PlayerPiece*>(kvp->getObject());
			if (playerPiece)
			{
				playerPiece->removeEventListener(PlayerPiece::EV_TYPE_DID_TRANSITION_OUT(), this);
				playerPiece->returnToPool();
				CC_SAFE_RELEASE(playerPiece);
			}
		}
        CC_SAFE_RELEASE_NULL(mPlayerPieces);
    }

	if (mCanvas)
	{
		for (int i = 0; i < kCanvasLen; ++i)
			CC_SAFE_RELEASE_NULL(mCanvas[i]);
	}

	if (mPathIndexes)
	{
		mNumPathIndexes = 0;
		delete[] mPathIndexes, mPathIndexes = NULL;
	}

	if (mPathTweener)
	{
		mPathTweener->setListener(NULL);
		CC_SAFE_RELEASE_NULL(mPathTweener);
	}

	if (mTeleportTweener)
	{
		mTeleportTweener->setListener(NULL);
		CC_SAFE_RELEASE_NULL(mTeleportTweener);
	}

	if (mKeyTweener)
	{
		mKeyTweener->setListener(NULL);
		CC_SAFE_RELEASE_NULL(mKeyTweener);
	}
    
    if (mIqTweener)
    {
        mIqTweener->setListener(NULL);
		CC_SAFE_RELEASE_NULL(mIqTweener);
    }
    
    if (mColorArrowClip)
    {
        mScene->removeFromJuggler(mColorArrowClip);
        CC_SAFE_RELEASE_NULL(mColorArrowClip);
    }

    setPuzzle(NULL);
    mScene->removeTweensWithTarget(this);
	CC_SAFE_RELEASE_NULL(mPreBatch);
	CC_SAFE_RELEASE_NULL(mPostBatch);
	CC_SAFE_RELEASE_NULL(mKeyNode);
	CC_SAFE_RELEASE_NULL(mShieldManager);
	CC_SAFE_RELEASE_NULL(mHighlighter);
    CC_SAFE_RELEASE_NULL(mIqTag);
    CC_SAFE_RELEASE_NULL(mIqTagNode);
	CC_SAFE_RELEASE_NULL(mAddQueue);
	CC_SAFE_RELEASE_NULL(mRemoveQueue);
    mIqLabel[kIQLabelIndexSML] = NULL;
    mIqLabel[kIQLabelIndexLGE] = NULL;
    mPathNotFound = NULL;
}

PuzzleBoard* PuzzleBoard::create(int category, Puzzle* puzzle, bool autorelease)
{
	PuzzleBoard *board = new PuzzleBoard(category);
	if (board && board->initWithPuzzle(puzzle))
    {
		if (autorelease)
			board->autorelease();
        return board;
    }
    CC_SAFE_DELETE(board);
    return NULL;
}

bool PuzzleBoard::initWithPuzzle(Puzzle* puzzle)
{
	CCAssert(puzzle, "PuzzleBoard::initWithPuzzle requires a non-null puzzle.");
	//mAdvanceable = true; // Leave our PuzzleController to advance us, rather than the scene.
    setPuzzle(puzzle);

    mPlayerPieces = new CCDictionary();
    mAddQueue = new CCArray();
    mRemoveQueue = new CCArray();

	for (int i = 0; i < kCanvasLen; ++i)
	{
		CCNode* node = new CCNode();
		//sprite->initWithSpriteFrame(mScene->spriteFrameByName("clear"));
		mCanvas[i] = node;
		addChild(node);
	}

	int numRows = mPuzzle->getNumRows(), numColumns = mPuzzle->getNumColumns();
	mPreBatch = new CCSpriteBatchNode();
	mPreBatch->initWithTexture(mScene->textureByName("playfield-atlas"), 4 * numRows * numColumns);

	mKeyNode = new CCNode();

	mPostBatch = new CCSpriteBatchNode();
	mPostBatch->initWithTexture(mScene->textureByName("playfield-atlas"), 4 * numRows * numColumns);

	mCanvas[kLowerCanvas]->addChild(mPreBatch);
	mCanvas[kLowerCanvas]->addChild(mKeyNode);
	mCanvas[kLowerCanvas]->addChild(mPostBatch);

    enableBatching(false);
	for (int i = 0; i < numRows; ++i)
	{
		for (int j = 0; j < numColumns; ++j)
		{
			int tileIndex = i * numColumns + j;
			TilePiece* tilePiece = TilePiece::getTilePiece(mPuzzle->tileAtIndex(tileIndex));
			tilePiece->retain();
			tilePiece->setDecorator(this);

			CCSize tileDims = tilePiece->tileDimensions();
			tilePiece->setPosition(ccp(j * tileDims.width, (numRows - (i + 1)) * tileDims.height));
			mTilePieces.push_back(tilePiece);
			//batchTilePiece(tilePiece);
		}
	}
    enableBatching(true);
    rebatchTilePieces();

	mTransitions.resize(numRows * numColumns, NULL);
	for (int i = 0; i < (int)mTransitions.size(); ++i)
		mTransitions[i] = new FloatTweener(0, this);

	//vec2 tileDims = getTileDimensions(), boardDims = getBoardDimensions();
	//mTileBatch->setPosition(ccp((mScene->getViewWidth() - boardDims.x) / 2 + tileDims.x / 2, (mScene->getViewHeight() - boardDims.y) / 2 + tileDims.y / 2));

	setCanvasScale(calculateCanvasScaler());

	CCArray* players = mPuzzle->getPlayers();
	CCObject* obj;
	CCARRAY_FOREACH(players, obj)
	{
		Player* player = static_cast<Player*>(obj);
		if (player)
			addPlayerPiece(player);
	}

	mShieldManager = new ShieldManager(CMGlobals::BOARD, puzzle->getNumColumns(), puzzle->getNumRows(), CCRectZero);
	mCanvas[kMidCanvas]->addChild(mShieldManager);

	mTeleportTweener = new ByteTweener(0, this, CMTransitions::LINEAR);
	mTeleportTweener->setTag(kByteTweenerTeleportTag);
	mTeleportTweener->reset(TileDecoration::kTeleportGlowOpacityMax, TileDecoration::kTeleportGlowOpacityMin, TileDecoration::kTeleportGlowDuration);

	mKeyTweener = new ByteTweener(0, this, CMTransitions::LINEAR);
	mKeyTweener->setTag(kByteTweenerKeyTag);
	mKeyTweener->reset(0, 255, 6.0f);
    
    CCArray* streakFrames = mScene->spriteFramesStartingWith("color-streak_");
    mColorArrowClip = MovieClip::createWithFrames(streakFrames, TileDecoration::kColorArrowFps, false);

	// Highlighter (for menus) + iqTag
	vec2 tileDims = getTileDimensions(), boardDims = getBoardDimensions();
	if (mTilePieces.size() > 0 && mTilePieces[0])
	{
		mHighlighter = new CCSprite();
		mHighlighter->initWithSpriteFrame(mScene->spriteFrameByName("puzzle-highlight"));
		mHighlighter->setPosition(ccp(boardDims.x / 2 - tileDims.x / 2, boardDims.y / 2 - tileDims.y / 2));
		mHighlighter->setScaleX((1.335f * boardDims.x) / mHighlighter->boundingBox().size.width);
		mHighlighter->setScaleY((1.41f * boardDims.y) / mHighlighter->boundingBox().size.height);
		mHighlighter->setVisible(false);
		mCanvas[kLowerCanvas]->addChild(mHighlighter, -1);
        
        mIqTag = new CCSprite();
        mIqTag->initWithSpriteFrame(mScene->spriteFrameByName("iq-tag"));
        mIqTag->setScaleX(-1);
        mIqTag->setPosition(ccp(7 - mIqTag->boundingBox().size.width / 2, -mIqTag->boundingBox().size.height / 2));
        mIqTag->setColor(CMUtils::uint2color3B(Puzzles::colorForIQ(mPuzzle->getIQ())));
        
        float iqTagOffsetX = 0;
        {
            CCLabelBMFont* iqLabel = TextUtils::createCommonSML("000\n000", 26, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
            iqLabel->setString(CMUtils::strConcatVal("IQ\n", mPuzzle->getIQ()).c_str());
            iqLabel->setPosition(ccp(
                                      iqTagOffsetX + (mIqTag->getPositionX() + 4),
                                      mIqTag->getPositionY() + (mIqTag->boundingBox().size.height - iqLabel->boundingBox().size.height) / 4));
            iqLabel->setColor(CMUtils::uint2color3B(0xffffff));
            iqLabel->setVisible(mIsMenuModeEnabled);
            mIqLabel[kIQLabelIndexSML] = iqLabel;
        }
        
        {
            CCLabelBMFont* iqLabel = TextUtils::createCommonLGE("000\n000", 26, kCCLabelAutomaticWidth, kCCTextAlignmentCenter, CCPointZero);
            iqLabel->setString(CMUtils::strConcatVal("IQ\n", mPuzzle->getIQ()).c_str());
            iqLabel->setPosition(ccp(
                                     iqTagOffsetX + (mIqTag->getPositionX() + 4),
                                     mIqTag->getPositionY() + (mIqTag->boundingBox().size.height - iqLabel->boundingBox().size.height) / 4));
            iqLabel->setColor(CMUtils::uint2color3B(0xffffff));
            iqLabel->setVisible(!mIsMenuModeEnabled);
            mIqLabel[kIQLabelIndexLGE] = iqLabel;
        }
        
        mIqTagNode = new CCNode();
        mIqTagNode->setPosition(ccp(-(tileDims.x / 2 - 2), boardDims.y - tileDims.y / 2));
        mIqTagNode->setAnchorPoint(ccp(0, 1));
        //mIqTagNode->setScaleX(2.25f);
        //mIqTagNode->setScaleY(3.0f);
        //mIqTagNode->setVisible(false);
        mIqTagNode->setScaleX(1.0f);
        mIqTagNode->setScaleY(1.33f);
        mIqTagNode->addChild(mIqTag);
        mIqTagNode->addChild(mIqLabel[kIQLabelIndexSML]);
        mIqTagNode->addChild(mIqLabel[kIQLabelIndexLGE]);
        mCanvas[kLowerCanvas]->addChild(mIqTagNode, -1);
        
        mIqTweener = new ByteTweener(255, this, CMTransitions::LINEAR);
        mIqTweener->setTag(kByteTweenerIqTag);
        updateIQText(puzzle);
	}

    //setContentSize(CCSizeMake(boardDims.x, boardDims.y));
    mScene->registerResDependent(this);
    mScene->registerLocalizable(this);
	return true; // You'll see clearly if anything fails.
}

void PuzzleBoard::setPuzzle(Puzzle* value)
{
	if (mPuzzle == value)
		return;
	if (mPuzzle)
	{
		mPuzzle->deregisterView(this);
		mPuzzle->removeEventListener(Puzzle::EV_TYPE_PLAYER_ADDED(), this);
		mPuzzle->removeEventListener(Puzzle::EV_TYPE_PLAYER_REMOVED(), this);
        mPuzzle->removeEventListener(Puzzle::EV_TYPE_FORCE_COMPLETE_ALL_MECHANICS(), this);
	}
	CC_SAFE_RETAIN(value);
	CC_SAFE_RELEASE(mPuzzle);
    mPuzzle = value;
	if (mPuzzle)
	{
		mPuzzle->registerView(this);
		mPuzzle->addEventListener(Puzzle::EV_TYPE_PLAYER_ADDED(), this);
		mPuzzle->addEventListener(Puzzle::EV_TYPE_PLAYER_REMOVED(), this);
        mPuzzle->addEventListener(Puzzle::EV_TYPE_FORCE_COMPLETE_ALL_MECHANICS(), this);
	}
}

vec2 PuzzleBoard::getTileDimensions(void) const
{
	if (mTilePieces.size() == 0)
        return CMVec2Zero;
    else
    {
		CCSize tileDims = mTilePieces[0]->tileDimensions();
		return cmv2(tileDims.width, tileDims.height);
    }
}

vec2 PuzzleBoard::getBoardDimensions(void) const
{
	if (mPuzzle == NULL || mTilePieces.size() == 0)
        return CMVec2Zero;
    else
    {
        vec2 tileDims = getTileDimensions();
        return cmv2(tileDims.x * mPuzzle->getNumColumns(), tileDims.y * mPuzzle->getNumRows());
    }
}

vec2 PuzzleBoard::getScaledTileDimensions(void)
{
    vec2 dims = getTileDimensions();
    dims.x *= getScaleX();
    dims.y *= getScaleY();
    return dims;
}

vec2 PuzzleBoard::getScaledBoardDimensions(void)
{
    vec2 dims = getBoardDimensions();
    dims.x *= getScaleX();
    dims.y *= getScaleY();
    return dims;
}

CCRect PuzzleBoard::getInvertedBoardBounds(void)
{
	if (mTilePieces.size() == 0)
        return CCRectZero;

	TilePiece* topLeftTile = mTilePieces[0];
	TilePiece* btmRightTile = mTilePieces[mTilePieces.size() - 1];

    
	CCPoint topLeft = topLeftTile->convertToWorldSpace(ccp(0, topLeftTile->boundingBox().size.height));
	CCPoint btmRight = btmRightTile->convertToWorldSpace(ccp(btmRightTile->boundingBox().size.width, 0));
    
    topLeft = mScene->getBaseSprite()->convertToNodeSpace(topLeft);
    btmRight = mScene->getBaseSprite()->convertToNodeSpace(btmRight);

    return CCRectMake(
                      topLeft.x,
                      topLeft.y,
                      btmRight.x - topLeft.x,
                      topLeft.y - btmRight.y);
}

CCRect PuzzleBoard::getGlobalBoardClippingBounds(void)
{
	if (mTilePieces.size() == 0)
        return CCRectZero;
    
	TilePiece* topLeftTile = mTilePieces[0];
	TilePiece* btmRightTile = mTilePieces[mTilePieces.size() - 1];
    
    CCRect topLeftBounds = topLeftTile->tileBounds();
    CCRect btmRightBounds = btmRightTile->tileBounds();
    
	CCPoint topLeft = mCanvas[kLowerCanvas]->convertToWorldSpace(ccp(topLeftBounds.origin.x, topLeftBounds.origin.y + topLeftBounds.size.height));
	CCPoint btmRight = mCanvas[kLowerCanvas]->convertToWorldSpace(ccp(btmRightBounds.origin.x + btmRightBounds.size.width, btmRightBounds.origin.y));
    
    CCRect bounds = CCRectMake(topLeft.x, topLeft.y - (topLeft.y - btmRight.y), btmRight.x - topLeft.x, topLeft.y - btmRight.y);
	/*bounds.setRect(
		bounds.origin.x,
		bounds.origin.y,
		bounds.size.width,
		bounds.size.height);*/
	return bounds;
}

CCRect PuzzleBoard::getIqTagGlobalBounds(void)
{
    CCPoint btmLeft = ccp(mIqTag->getPositionX() - mIqTag->boundingBox().size.width / 2,
                          mIqTag->getPositionY() - mIqTag->boundingBox().size.height / 2);
    btmLeft = mIqTagNode->convertToWorldSpace(btmLeft);

    CCPoint topRight = ccp(mIqTag->getPositionX() + mIqTag->boundingBox().size.width / 2,
                           mIqTag->getPositionY() + mIqTag->boundingBox().size.height / 2);
    topRight = mIqTagNode->convertToWorldSpace(topRight);
    
    return CCRectMake(btmLeft.x,
                      btmLeft.y,
                      topRight.x - btmLeft.x,
                      topRight.y - btmLeft.y);
}

void PuzzleBoard::updateBounds(const CCPoint& pos)
{
    setPosition(pos);
    
    if (mShieldManager)
	{
		CCRect viewableRegion = getGlobalBoardClippingBounds();
		viewableRegion.setRect(
			viewableRegion.origin.x,
			viewableRegion.origin.y,
			viewableRegion.size.width,
			viewableRegion.size.height + mScene->getViewScale().y * getTileDimensions().y);
		mShieldManager->setViewableRegion(viewableRegion);
	}
}

void PuzzleBoard::refreshColorScheme(void)
{
    for(std::vector<TilePiece*>::iterator it = mTilePieces.begin(); it != mTilePieces.end(); ++it)
    {
        TilePiece* tilePiece = *it;
        if (tilePiece)
            tilePiece->syncWithData();
    }
    
    CCDictElement* kvp;
    CCDICT_FOREACH(mPlayerPieces, kvp)
    {
        PlayerPiece* playerPiece = static_cast<PlayerPiece*>(kvp->getObject());
        if (playerPiece)
            playerPiece->refreshAesthetics();
    }
}

void PuzzleBoard::resolutionDidChange(void)
{
    if (mIsMenuModeEnabled)
    {
        for(std::vector<TilePiece*>::iterator it = mTilePieces.begin(); it != mTilePieces.end(); ++it)
        {
            TilePiece* tilePiece = *it;
            if (tilePiece)
                tilePiece->updateMenuScale();
        }
    }
    
    setCanvasScale(calculateCanvasScaler());
    updateBounds(getPosition());
}

void PuzzleBoard::localeDidChange(const char* fontKey, const char* FXFontKey)
{
    updateIQText(mPuzzle);
}

void PuzzleBoard::setTransitioning(bool value)
{
	if (mPuzzle)
	{
		if (value)
			mPuzzle->pause();
		else
			mPuzzle->resume();
	}

	mIsTransitioning = value;
    
    if (mIsTransitioning)
        dispatchEvent(EV_TYPE_BG_EXPOSURE_WILL_BEGIN(), this); // EV_TYPE_DID_TRANSITION_IN will toggle listeners off
}

void PuzzleBoard::transitionIqTag(bool inwards, float duration, float delay)
{
    if (mIqTag && mIqLabel[kIQLabelIndexLGE] && mIqTweener)
    {
        if (inwards)
        {
            mIqTag->setOpacity(0);
            mIqLabel[kIQLabelIndexLGE]->setOpacity(0);
            mIqTweener->reset(0, 255, duration, delay);
        }
        else
        {
            mIqTag->setOpacity(255);
            mIqLabel[kIQLabelIndexLGE]->setOpacity(255);
            mIqTweener->reset(255, 0, duration, delay);
        }
    }
    
}

void PuzzleBoard::transition(int tag, float duration, float columnDelay, float rowDelay, TRANS_FUNC transition)
{
	if (mPuzzle == NULL || duration == 0)
		return;

	CCAssert(mTransitions.size() == mTilePieces.size(), "PuzzleBoard::transition - invalid internal state.");

	int numCols = mPuzzle->getNumColumns();
	int columnIter = 0, rowIter = 0;
	float deltaX = mScene->getFullscreenWidth();
	for (int i = 0; i < (int)mTransitions.size(); ++i)
	{
		float from = mTilePieces[i]->getPositionX();
		float to = from + deltaX;
		float delay = rowDelay * rowIter + columnDelay * columnIter;
		mTransitions[i]->reset(from, to, duration, delay);
		mTransitions[i]->setTag(tag + i);
		mTransitions[i]->setTransition(transition);

		if (++columnIter == numCols)
        {
            columnIter = 0;
            ++rowIter;
        }
	}
}

void PuzzleBoard::transitionIn(float duration, float columnDelay, float rowDelay)
{
	if (isTransitioning() || mPuzzle == NULL)
		return;

	setTransitioning(true);
	setVisible(true);
	unbatchAllTilePieces();

	int numRows = mPuzzle->getNumRows(), numCols = mPuzzle->getNumColumns();
	for (int i = 0; i < numRows; ++i)
    {
		for (int j = 0; j < numCols; ++j)
        {
            int tileIndex = i * numCols + j;
            TilePiece* tilePiece = mTilePieces[tileIndex];
			CCSize tileDims = tilePiece->tileDimensions();
			tilePiece->setPosition(ccp(
				j * tileDims.width - mScene->getFullscreenWidth(),
				(numRows - (i + 1)) * tileDims.height));
            tilePiece->setAestheticState(TilePiece::EDGE);
			batchTilePiece(tilePiece); // Re-order Z-values from front to back
        }
    }

	transition(kTransitionInTag, duration, columnDelay, rowDelay, CMTransitions::EASE_OUT);

	mLastPlayerTransitionDelay = duration + numCols * columnDelay + numRows * rowDelay - 0.25f;
	if (mPlayerPieces)
    {
		CCDictElement* kvp;
		CCDICT_FOREACH(mPlayerPieces, kvp)
		{
			PlayerPiece* playerPiece = static_cast<PlayerPiece*>(kvp->getObject());
			if (playerPiece)
				playerPiece->transitionIn(0.25f, mLastPlayerTransitionDelay);
		}
    }
    
    transitionIqTag(true, 1.0f, mLastPlayerTransitionDelay);
}

void PuzzleBoard::transitionOut(float duration, float columnDelay, float rowDelay)
{
	if (isTransitioning() || mPuzzle == NULL)
		return;

    setTransitioning(true);
	unbatchAllTilePieces();

	int numRows = mPuzzle->getNumRows(), numCols = mPuzzle->getNumColumns();
    for (int i = 0; i < numRows; ++i)
    {
        for (int j = numCols - 1; j >= 0; --j)
        {
            int tileIndex = i * numCols + j;
            TilePiece* tilePiece = mTilePieces[tileIndex];
			CCSize tileDims = tilePiece->tileDimensions();
            tilePiece->setPosition(ccp(
				j * tileDims.width,
				(numRows - (i + 1)) * tileDims.height));
            tilePiece->setAestheticState(TilePiece::EDGE);
			batchTilePiece(tilePiece); // Re-order Z-values from back to front
        }
    }

	transition(kTransitionOutTag, duration, columnDelay, rowDelay, CMTransitions::EASE_IN);

	if (mPlayerPieces)
    {
		CCDictElement* kvp;
		CCDICT_FOREACH(mPlayerPieces, kvp)
		{
			PlayerPiece* playerPiece = static_cast<PlayerPiece*>(kvp->getObject());
			if (playerPiece)
				playerPiece->transitionOut(0.25f);
		}
    }
    
    transitionIqTag(false, 0.5f, 0.0f);
}

void PuzzleBoard::setCanvasScale(float value)
{
	setScale(value);
	setScaleX(value * kTileScaleX);
	mCanvas[kLowerPlayerCanvas]->setScaleX(1.0f / kTileScaleX);
	mCanvas[kUpperPlayerCanvas]->setScaleX(1.0f / kTileScaleX);
}

float PuzzleBoard::calculateCanvasScaler(void) const
{
	vec2 boardDims = getBoardDimensions();

#if 1

    float scaler = IS_TABLET ? 0.72f : 0.81f;
	float maxScale = MIN(scaler * mScene->getViewWidth() / boardDims.x, scaler * mScene->getViewHeight() / boardDims.y); // 0.74f

	//#ifdef CHEEKY_MOBILE
	//	maxScale /= CCDirector::sharedDirector()->getContentScaleFactor();
	//#endif

	return maxScale;

#else

	#ifdef CHEEKY_MOBILE

		#ifdef __ANDROID__
			//float maxScale = MAX(0.55f * mScene->getViewWidth() / boardDims.x, 0.75f * mScene->getViewHeight() / boardDims.y);
			float maxScale = MIN(0.75f * mScene->getViewWidth() / boardDims.x, 0.75f * mScene->getViewHeight() / boardDims.y);
			maxScale /= CCDirector::sharedDirector()->getContentScaleFactor();
			return maxScale;
		#else
			float maxScale = MODE_8x6
					? MAX(0.7f * mScene->getViewportWidth() / boardDims.x, 0.9f * mScene->getViewportHeight() / boardDims.y)
					: MAX(0.55f * mScene->getViewportWidth() / boardDims.x, 0.75f * mScene->getViewportHeight() / boardDims.y);
			// We've already been scaled by the scene, so maxScale is only trying to scale to our aspect ratio. Therefore
			// we undo the content scale which the scene already accounts for.
			maxScale /= CCDirector::sharedDirector()->getContentScaleFactor();
			return maxScale;
		#endif
	#else
		float maxScale = MAX(0.55f * mScene->getViewWidth() / boardDims.x, 0.75f * mScene->getViewHeight() / boardDims.y);
		return maxScale;
	#endif

#endif

	/*vec2 viewport = cmv2(mScene->getViewportWidth(), mScene->getViewportHeight());
	vec2 view = cmv2(mScene->getViewWidth(), mScene->getViewHeight());

	if (viewport.x < view.x - 1)
	{
		vec2 boardDims = getBoardDimensions();
		return MIN(mScene->getMaximizingContentScaleFactor(), 0.8f * mScene->getViewHeight() / boardDims.y);
	}
	else
		return 1.0f;*/
}

GLubyte PuzzleBoard::decoratorValueForKey(uint key)
{
    GLubyte value = 0;
    switch (key)
    {
		case TilePiece::kTDKTeleport:
			value = mTeleportTweener ? mTeleportTweener->getTweenedValue() : 255;
            break;
        case TilePiece::kTDKPainter:
            value = mColorArrowClip ? (GLubyte)mColorArrowClip->getCurrentFrame() : 0;
            break;
		case TilePiece::kTDKKey:
            value = mKeyTweener ? mKeyTweener->getTweenedValue() : 255;
            break;
    }

    return value;
}

void PuzzleBoard::decorationDidChange(TilePiece* tilePiece)
{
	tilePieceKeyStateDidChange(tilePiece);
}

void PuzzleBoard::enableMenuMode(bool enable, bool useMenuScale)
{
    enableBatching(true);
    unbatchAllTilePieces();
    enableBatching(false);
    
	for(std::vector<TilePiece*>::size_type i = 0; i < mTilePieces.size(); ++i)
		mTilePieces[i]->enableMenuMode(enable);

    if (mPlayerPieces)
    {
		CCDictElement* kvp;
		CCDICT_FOREACH(mPlayerPieces, kvp)
		{
			PlayerPiece* playerPiece = static_cast<PlayerPiece*>(kvp->getObject());
			if (playerPiece)
				playerPiece->enableMenuMode(enable);
		}
    }
    
    if (mIqLabel[kIQLabelIndexSML])
        mIqLabel[kIQLabelIndexSML]->setVisible(enable);
    if (mIqLabel[kIQLabelIndexLGE])
        mIqLabel[kIQLabelIndexLGE]->setVisible(!enable);
    
    if (mIqTagNode)
    {
        mIqTagNode->setScaleX(enable && useMenuScale ? 2.15f : 1.0f);
        mIqTagNode->setScaleY(enable && useMenuScale ? 3.0f : 1.33f);
        //mIqTagNode->setVisible(enable);
    }

	setCanvasScale(enable && useMenuScale ? 1.0f : calculateCanvasScaler());
    mIsMenuModeEnabled = enable;
    
    enableBatching(true);
    rebatchTilePieces();
}

void PuzzleBoard::setData(Puzzle* puzzle)
{
	if (mPlayerPieces == NULL)
        return;

	if (mShieldManager)
        mShieldManager->withdrawAll(false);
    
    forceCompleteActiveAnimations();

    enableBatching(true);
    unbatchAllTilePieces();
    enableBatching(false);
    
	for(std::vector<TilePiece*>::size_type i = 0; i < mTilePieces.size(); ++i)
		mTilePieces[i]->setData(NULL);

	setPuzzle(puzzle);

    if (mPuzzle)
    {
        if (mIqTag)
            mIqTag->setColor(CMUtils::uint2color3B(Puzzles::colorForIQ(mPuzzle->getIQ())));
        updateIQText(mPuzzle);
        
		for(std::vector<TilePiece*>::size_type i = 0; i < mTilePieces.size(); ++i)
			mTilePieces[i]->setData(mPuzzle->tileAtIndex((int)i));

        clearPlayerPieces(mIsMenuModeEnabled ? 0 : 0.25f);
        CCArray* players = mPuzzle->getPlayers();
		CCObject* obj;
		CCARRAY_FOREACH(players, obj)
		{
			Player* player = static_cast<Player*>(obj);
			if (player)
				addPlayerPiece(player);
		}

        // Hack
        if (isTransitioning())
        {
			CCDictElement* kvp;
			CCDICT_FOREACH(mPlayerPieces, kvp)
			{
				PlayerPiece* playerPiece = static_cast<PlayerPiece*>(kvp->getObject());
				if (playerPiece && playerPiece->getPlayer() && !mRemoveQueue->containsObject(playerPiece->getPlayer()))
					playerPiece->transitionIn(0.25f, mLastPlayerTransitionDelay);
			}
        }
    }
    else
    {
        for(std::vector<TilePiece*>::size_type i = 0; i < mTilePieces.size(); ++i)
			mTilePieces[i]->setData(NULL);
        CCDictElement* kvp;
		CCDICT_FOREACH(mPlayerPieces, kvp)
		{
			PlayerPiece* playerPiece = static_cast<PlayerPiece*>(kvp->getObject());
			if (playerPiece)
				playerPiece->setData(NULL);
		}
    }
    
    enableBatching(true);
    rebatchTilePieces();

    //syncWithData();
}

void PuzzleBoard::syncWithData()
{
	if (mPuzzle)
	{
		for(std::vector<TilePiece*>::size_type i = 0; i < mTilePieces.size(); ++i)
			mTilePieces[i]->syncWithData();
	}
}

void PuzzleBoard::setHighlightColor(const ccColor3B& value)
{
	if (mHighlighter)
		mHighlighter->setColor(value);
}

void PuzzleBoard::enableHighlight(bool enable)
{
	if (mHighlighter)
		mHighlighter->setVisible(enable);
}

//#define PUZZLE_BOARD_FUTURE_PATH
void PuzzleBoard::enablePath(Player* player, const Coord* path, int numSteps)
{
	if (mPuzzle == NULL || player == NULL || path == NULL || numSteps <= 0 || numSteps > mPuzzle->getNumTiles())
		return;

	disablePath();

	if (mPathIndexes == NULL)
		mPathIndexes = new int[mPuzzle->getNumTiles()];
	mNumPathIndexes = 0;

	if (mPathTweener == NULL)
	{
		mPathTweener = new ByteTweener(0, this, CMTransitions::EASE_IN);
		mPathTweener->setTag(kByteTweenerPathTag);
	}
	mScene->removeFromJuggler(mPathTweener);
	mPathTweener->reset(255, 0, 0.25f, 0.25f);
	mScene->addToJuggler(mPathTweener);

	int numColorMagics = player->getNumColorMagicMoves() - (player->isMoving() ? 1 : 0);
	for (int i = 0; i < numSteps; ++i, ++mNumPathIndexes, --numColorMagics)
	{
		int index = mPuzzle->pos2Index(path[i]);
		mPathIndexes[i] = index;

		TilePiece* tilePiece = getTilePieceAtIndex(index);
		CCAssert(tilePiece, "PuzzleBoard:: bad path in enablePath.");
		tilePiece->enablePath(numColorMagics > 0);
	}

/*
#ifdef PUZZLE_BOARD_FUTURE_PATH
	Coord prevMove = cmc(path[0].x - player->getPosition().x, path[0].y - player->getPosition().y);
	int numColorMagics = player->getNumColorMagicMoves();
	for (int i = 0; i < numSteps; ++i, --numColorMagics)
	{
		int index = mPuzzle->pos2Index(path[i]);
		TilePiece* tilePiece = getTilePieceAtIndex(index);
		CCAssert(tilePiece, "PuzzleBoard:: bad path in enablePath.");
		Coord moveVector = (i + 1 < numSteps)
			? cmc(path[i+1].x - path[i].x, path[i+1].y - path[i].y)
			: prevMove;
		tilePiece->enablePath(Player::coord2Orientation(moveVector), numColorMagics > 0);
		prevMove = moveVector;
	}
#else
	Coord playerPos = player->getPosition();
	int numColorMagics = player->getNumColorMagicMoves();
	for (int i = 0; i < numSteps; ++i, --numColorMagics)
	{
		int index = mPuzzle->pos2Index(path[i]);
		TilePiece* tilePiece = getTilePieceAtIndex(index);
		CCAssert(tilePiece, "PuzzleBoard:: bad path in enablePath.");
		Coord moveVector = (i > 0)
			? cmc(path[i].x - path[i-1].x, path[i].y - path[i-1].y)
			: cmc(path[i].x - playerPos.x, path[i].y - playerPos.y);
		tilePiece->enablePath(Player::coord2Orientation(moveVector), numColorMagics > 0);
	}
#endif
*/
}

void PuzzleBoard::disablePath(void)
{
	if (mPathIndexes)
	{
		for (int i = 0; i < mNumPathIndexes; ++i)
			mTilePieces[mPathIndexes[i]]->disablePath();
	}
    
    hidePathNotFound();
}

void PuzzleBoard::displayPathNotFound(int tileIndex)
{
    TilePiece* tilePiece = this->getTilePieceAtIndex(tileIndex);
    if (tilePiece)
    {
        if (mPathNotFound == NULL)
        {
            mPathNotFound = CCSprite::createWithSpriteFrame(mScene->spriteFrameByName("tile-error"));
            mPathNotFound->setVisible(false);
            mCanvas[kLowerCanvas]->addChild(mPathNotFound);
        }
        
        mPathNotFoundTimer = 0;
        mPathNotFound->setPosition(tilePiece->getPosition());
        mPathNotFound->setOpacity(255);
        mPathNotFound->setVisible(true);
    }
}

void PuzzleBoard::hidePathNotFound(void)
{
    if (mPathNotFound)
        mPathNotFound->setVisible(false);
}

TilePiece* PuzzleBoard::getTilePieceAtIndex(int index) const
{
	if (index < 0 || index >= (int)mTilePieces.size())
        return NULL;
    else
        return mTilePieces[index];
}

void PuzzleBoard::tilePieceKeyStateDidChange(TilePiece* tilePiece)
{
	if (tilePiece == NULL)
		return;

	if (!tilePiece->isBatchable())
	{
		if (mKeyNode->getChildrenCount() == 0)
			rebatchTilePieces();
		else
		{
			tilePiece->removeFromParent();
			mKeyNode->addChild(tilePiece);
		}
	}
	else
		rebatchTilePieces();
}

void PuzzleBoard::batchTilePiece(TilePiece* tilePiece)
{
	if (tilePiece == NULL || !isBatchingEnabled())
		return;

	tilePiece->removeFromParent();

	if (!tilePiece->isBatchable())
		mKeyNode->addChild(tilePiece);
	else if (mKeyNode->getChildrenCount() == 0)
		mPreBatch->addChild(tilePiece);
	else
		mPostBatch->addChild(tilePiece);
}

void PuzzleBoard::unbatchAllTilePieces(void)
{
    if (!isBatchingEnabled())
        return;
    
	if (mPreBatch)
		mPreBatch->removeAllChildren();
	if (mPostBatch)
		mPostBatch->removeAllChildren();
	if (mKeyNode)
		mKeyNode->removeAllChildren();
}

void PuzzleBoard::rebatchTilePieces(void)
{
    if (!isBatchingEnabled())
        return;
    
	unbatchAllTilePieces();
    
	CCSpriteBatchNode* batch = mPreBatch;
	for(std::vector<TilePiece*>::iterator it = mTilePieces.begin(); it != mTilePieces.end(); ++it)
	{
		if (*it == NULL)
			continue;
        
		if (!(*it)->isBatchable())
		{
			batch = mPostBatch;
			mKeyNode->addChild(*it);
		}
		else
			batch->addChild(*it);
	}
}

void PuzzleBoard::addPlayerPiece(Player* player)
{
	if (mPuzzle && player && mPlayerPieces && !mPlayerPieces->objectForKey(player->getID()))
    {
        if (mIsLocked)
			mAddQueue->addObject(player);
        else
        {
			vec2 tileDims = getTileDimensions();
			player->setMoveDimensions(cmv2(tileDims.x * kTileScaleX, tileDims.y));
			player->setViewOffset(cmc(0, mPuzzle->getNumRows()-1));
			PlayerPiece* playerPiece = PlayerPiece::getPlayerPiece(player);
			playerPiece->retain();
			mPlayerPieces->setObject(playerPiece, player->getID());
			mCanvas[kLowerPlayerCanvas]->addChild(playerPiece);
			refreshPlayerZValues();
        }
    }
}

void PuzzleBoard::removePlayerPiece(Player* player, float transitionOutDuration)
{
	if (player && mPlayerPieces && mPlayerPieces->objectForKey(player->getID()))
    {
        if (mIsLocked)
		{
			if (!mRemoveQueue->containsObject(player))
				mRemoveQueue->addObject(player);
		}
        else
        {
			PlayerPiece* playerPiece = static_cast<PlayerPiece*>(mPlayerPieces->objectForKey(player->getID()));

			if (playerPiece)
			{
				mPlayerPieces->removeObjectForKey(player->getID());
				playerPiece->addEventListener(PlayerPiece::EV_TYPE_DID_TRANSITION_OUT(), this);
				playerPiece->transitionOut(transitionOutDuration);
			}
        }
    }
}

void PuzzleBoard::clearPlayerPieces(float transitionOutDuration)
{
	if (mPlayerPieces)
	{
		CCArray* keys = mPlayerPieces->allKeys();
		if (keys)
		{
			CCObject* obj;
			CCARRAY_FOREACH(keys, obj)
			{
				CCInteger* key = static_cast<CCInteger*>(obj);
				if (key)
				{
					PlayerPiece *playerPiece = static_cast<PlayerPiece*>(mPlayerPieces->objectForKey(key->getValue()));
					if (playerPiece && playerPiece->getPlayer())
						removePlayerPiece(playerPiece->getPlayer(), transitionOutDuration);
				}
			}
		}
	}
}

void PuzzleBoard::refreshPlayerZValues(void)
{
	if (mPuzzle == NULL || mPlayerPieces == NULL || mPlayerPieces->count() == 0)
		return;

    // Order players relative to shields.
	CCDictElement* kvp;
	CCDICT_FOREACH(mPlayerPieces, kvp)
	{
		Player* player = mPuzzle->getPlayerForID((int)kvp->getIntKey());
		PlayerPiece* playerPiece = static_cast<PlayerPiece*>(kvp->getObject());
		if (playerPiece && player)
		{
			playerPiece->removeFromParent();

			Coord currentPos = player->getPosition();
			int currentTileIndex = mPuzzle->pos2Index(currentPos);
			int currentShieldIndex = mPuzzle->pos2Index(cmc(currentPos.x, currentPos.y-2));

			PuzzleTile* tile = mPuzzle->tileAtIndex(currentTileIndex);
			if (tile == NULL || tile->isModified(PuzzleTile::kTMShielded))
			{
				mCanvas[kLowerPlayerCanvas]->addChild(playerPiece);
				continue;
			}

			Coord queuedMove = player->getQueuedMove();
			bool isPlayerMoving = queuedMove.x != 0 || queuedMove.y != 0;

			if (!isPlayerMoving)
			{
				tile = mPuzzle->tileAtIndex(currentShieldIndex);
				if (tile && tile->isModified(PuzzleTile::kTMShielded))
				{
					mCanvas[kUpperPlayerCanvas]->addChild(playerPiece);
					continue;
				}
			}
			else
			{
				Coord nextPos = player->getIsMovingTo();
				int nextTileIndex = mPuzzle->pos2Index(nextPos);
				int nextShieldIndex = mPuzzle->pos2Index(cmc(nextPos.x, nextPos.y-2));

				PuzzleTile* currentShieldTile = mPuzzle->tileAtIndex(currentShieldIndex);
				PuzzleTile* nextTile = mPuzzle->tileAtIndex(nextTileIndex);
				PuzzleTile* nextShieldTile = mPuzzle->tileAtIndex(nextShieldIndex);

				if ( (nextTile == NULL || !nextTile->isModified(PuzzleTile::kTMShielded))
					&&	(	   (currentShieldTile && currentShieldTile->isModified(PuzzleTile::kTMShielded)) 
							|| (nextShieldTile && nextShieldTile->isModified(PuzzleTile::kTMShielded))
						)
				   )
				{
					mCanvas[kUpperPlayerCanvas]->addChild(playerPiece);
					continue;
				}
			}
			
			mCanvas[kLowerPlayerCanvas]->addChild(playerPiece);	
		}
	}
    
    // Order players relative to each other. There can only be two players max, so the search is simplifed.
    CCDictElement* kvpOuter, *kvpInner;
    CCDICT_FOREACH(mPlayerPieces, kvpOuter)
    {
        Player* outerPlayer = mPuzzle->getPlayerForID((int)kvpOuter->getIntKey());
		PlayerPiece* outerPlayerPiece = static_cast<PlayerPiece*>(kvpOuter->getObject());
        
        CCDICT_FOREACH(mPlayerPieces, kvpInner)
        {
            Player* innerPlayer = mPuzzle->getPlayerForID((int)kvpInner->getIntKey());
            PlayerPiece* innerPlayerPiece = static_cast<PlayerPiece*>(kvpInner->getObject());
            
            if (outerPlayerPiece != innerPlayerPiece)
            {
                CCNode* outerParent = outerPlayerPiece->getParent(), *innerParent = innerPlayerPiece->getParent();
                if (outerParent != NULL && outerParent == innerParent)
                {
                    if (outerPlayer->getPosition().y > innerPlayer->getPosition().y)
                    {
                        if (outerPlayerPiece->getOrderOfArrival() < innerPlayerPiece->getOrderOfArrival())
                        {
                            outerPlayerPiece->removeFromParent();
                            outerParent->addChild(outerPlayerPiece);
                        }
                    }
                    else if (outerPlayer->getPosition().y == innerPlayer->getPosition().y)
                    {
                        if (outerPlayer->getType() == Player::MIRRORED_MATE && innerPlayer->getType() == Player::HUMAN_PLAYER &&
                            outerPlayerPiece->getOrderOfArrival() < innerPlayerPiece->getOrderOfArrival())
                        {
                            outerPlayerPiece->removeFromParent();
                            outerParent->addChild(outerPlayerPiece);
                        }
                    }
                }
            }
        }
        
    }
}

int PuzzleBoard::getEdgeStatus(int tileIndex)
{
    int edgeStatus = PuzzleEffects::NONE;
    if (mPuzzle)
    {
        int numCols = mPuzzle->getNumColumns(), numRows = mPuzzle->getNumRows();
        if (numCols > 0 && numRows > 0)
        {
            if (tileIndex % numCols == 0) // Left edge
                edgeStatus |= PuzzleEffects::LEFT;
            else if (tileIndex % numCols == numCols-1) // Right edge
                edgeStatus |= PuzzleEffects::RIGHT;
            
            if (tileIndex < numCols) // Top edge
                edgeStatus |= PuzzleEffects::TOP;
            else if (tileIndex >= (numRows - 1) * numCols) // Bottom edge
                edgeStatus |= PuzzleEffects::BOTTOM;
        }
    }
    
    return edgeStatus;
}

void PuzzleBoard::setIQLabelStr(std::string str)
{
    if (mIqLabel[kIQLabelIndexSML])
        mIqLabel[kIQLabelIndexSML]->setString(str.c_str());
    if (mIqLabel[kIQLabelIndexLGE])
        mIqLabel[kIQLabelIndexLGE]->setString(str.c_str());
}

void PuzzleBoard::updateIQText(const Puzzle* puzzle)
{
    if (mIqLabel[kIQLabelIndexSML] && mIqLabel[kIQLabelIndexLGE] && puzzle)
    {
        std::string iqPrefix;
        Localizer::LocaleType locale = mScene->getLocale();
        switch (locale)
        {
            case Localizer::ES:
                iqPrefix = "CI\n";
                break;
            case Localizer::FR:
            case Localizer::IT:
                iqPrefix = "QI\n";
                break;
            default:
                iqPrefix = "IQ\n";
                break;
        }
        
        setIQLabelStr(CMUtils::strConcatVal(iqPrefix.c_str(), puzzle->getIQ()));
    }
}

void PuzzleBoard::puzzleSoundShouldPlay(const char* soundName)
{
	if (soundName)
		mScene->playSound(soundName);
}

void PuzzleBoard::puzzlePlayerWillMove(Player* player)
{
	refreshPlayerZValues();
}

void PuzzleBoard::puzzlePlayerDidMove(Player* player)
{
	refreshPlayerZValues();
}

void PuzzleBoard::puzzleShieldDidDeploy(int tileIndex)
{
	if (mShieldManager == NULL)
		return;

    TilePiece* tilePiece = mTilePieces[tileIndex];

//	vec2 boardDims = getBoardDimensions();
//	CCRect topLeftBounds = CMUtils::boundsInSpace(mCanvas[kMidCanvas], mTilePieces[0]);
//	CCRect boardBounds = CCRectMake(
//		topLeftBounds.origin.x,
//		(topLeftBounds.origin.y + topLeftBounds.size.height) - boardDims.y,
//		boardDims.x,
//		boardDims.y);

	Shield* shield = mShieldManager->addShield(tileIndex, tileIndex, tilePiece->getPosition());
	if (shield)
	{
		shield->deploy();
		refreshPlayerZValues();
	}
}

void PuzzleBoard::puzzleShieldWasWithdrawn(int tileIndex)
{
	if (mShieldManager == NULL)
        return;

    int shieldIndex = tileIndex;
	Shield* shield = mShieldManager->shieldForKey(shieldIndex);
    if (shield)
	{
		shield->withdraw();
		refreshPlayerZValues();
	}
}

const int kTileRotatorEdgeIndexes[] = { 1, 7, 8 };
const int kTileRotatorIndexes[] = { 0, 3, 5, 6, 7, 4, 2, 1 };
void PuzzleBoard::puzzleTilesShouldRotate(const int2d& tileIndexes)
{
	if (mPuzzle == NULL)
		return;

	std::vector<TilePiece*> rotatePieces;
	rotatePieces.resize(8, NULL);
    for (int i = 0; i < 8; ++i)
		rotatePieces[kTileRotatorIndexes[i]] = mTilePieces[tileIndexes.at(0, i + 1)];

	// Present relevant tiles in the row above as edge tiles while animation is active
	for (int i = 0; i < 3; ++i)
	{
		//int edgeIndex = tileIndexes.at(0, kTileRotatorEdgeIndexes[i]) - (i == 0 ? 0 : mPuzzle->getNumColumns());
		int edgeIndex = tileIndexes.at(0, kTileRotatorEdgeIndexes[i]) - mPuzzle->getNumColumns();
		if (mPuzzle->isValidIndex(edgeIndex))
			mTilePieces[edgeIndex]->setAestheticState(TilePiece::EDGE);
	}

	TilePiece* centerPiece = mTilePieces[tileIndexes.at(0, 0)];
	CCPoint pos = mCanvas[kUpperCanvas]->convertToNodeSpace(mCanvas[kLowerCanvas]->convertToWorldSpace(centerPiece->getPosition()));
	CCRect bounds = centerPiece->boundingBox();
	bounds = CCRectMake(
		pos.x - bounds.size.width / 2,
		pos.y - bounds.size.height / 2,
		bounds.size.width,
		bounds.size.height);
	CCPoint origin = ccp(bounds.getMidX(), bounds.getMidY());
    
    CCRect boardBounds = getGlobalBoardClippingBounds();
    
    // Determine the rotator's EdgeStatus
    int edgeStatus = PuzzleEffects::NONE;
    {
        int tileIndex = tileIndexes.at(0, 1); // Skip first tile - it's the center tile
        edgeStatus |= getEdgeStatus(tileIndex);
        edgeStatus |= getEdgeStatus(tileIndex + 2);
        edgeStatus |= getEdgeStatus(tileIndex + 2 * mPuzzle->getNumColumns());
    }
    
	TileRotator* tileRotator = TileRotator::getTileRotator(origin, boardBounds, rotatePieces);
	tileRotator->retain();
	tileRotator->setDecorator(this);
	tileRotator->setListener(this);
    tileRotator->setEdgeStatus(edgeStatus);
#if DEBUG
    std::vector<TileRotator*>::iterator findIt = std::find(mAnimatingRotators.begin(), mAnimatingRotators.end(), tileRotator);
    CCAssert(findIt == mAnimatingRotators.end(), "PuzzleBoard: duplicate rotator found.");
#endif
    mAnimatingRotators.push_back(tileRotator);
	
	mScene->addToJuggler(tileRotator);
	tileRotator->animate(0.9f);

	int zOrder = 1;
	if (mPuzzle->isConveyorBeltActive() && mPuzzle->getConveyorBeltDir().x != 0)
	{
		int1d* beltIndexes = mPuzzle->getConveyorBeltIndexes();
		if (beltIndexes)
		{
			if (mPuzzle->rowForIndex(beltIndexes->at(0)) > mPuzzle->rowForIndex(tileIndexes.at(0, 0)))
				zOrder = 0;
		}
	}

	mCanvas[kUpperCanvas]->addChild(tileRotator, zOrder);

    // Add players that are positioned on the rotated tiles.
	CCArray* players = mPuzzle->getPlayers();
	if (players)
	{
		CCObject* obj;
		CCARRAY_FOREACH(players, obj)
		{
			Player* player = static_cast<Player*>(obj);
			if (player)
			{
				int playerIndex = mPuzzle->pos2Index(player->getPosition());

				// Pivot piece
				if (playerIndex == tileIndexes.at(0, 0))
				{
					PlayerPiece* playerPiece = addPlayerToEffect(player);
					if (playerPiece)
					{
						tileRotator->addPivot(mTilePieces[tileIndexes.at(0, 0)], playerPiece);
						mTilePieces[tileIndexes.at(0, 0)]->setVisible(false);
					}
				}

				// Non-pivot pieces
				for (int i = 0; i < 8; ++i)
				{
					if (playerIndex == tileIndexes.at(0, i + 1))
					{
						PlayerPiece* playerPiece = addPlayerToEffect(player);
						if (playerPiece)
							tileRotator->addPlayerPiece(kTileRotatorIndexes[i], playerPiece); // Rotator takes ownership of playerPiece
					}
				}
			}
		}
	}

    for (int i = 0; i < 8; ++i)
		mTilePieces[tileIndexes.at(0, i + 1)]->setVisible(false);
    
    dispatchEvent(EV_TYPE_BG_EXPOSURE_WILL_BEGIN(), this);
}

void PuzzleBoard::puzzleTileSwapWillBegin(const int2d& swapIndexes, bool isCenterValid)
{
    std::vector<TilePiece*> swapPieces;
	swapPieces.resize(swapIndexes.count(), NULL);
	for (int i = 0; i < swapIndexes.w(); ++i)
    {
		for (int j = 0; j < swapIndexes.h(); ++j)
        {
            // Skip all invalid tiles.
			if (swapIndexes.at(i, j) == -1)
                continue;
			swapPieces[i * swapIndexes.h() + j] = mTilePieces[swapIndexes.at(i, j)];
        }
    }

	vec4 swapOrigins = CMVec4Zero;
    for (int i = 0; i < 2; ++i)
    {
		TilePiece* centerPiece = mTilePieces[swapIndexes.at(i, 4)];
		if (i == 0)
		{
			swapOrigins.x = centerPiece->getPositionX();
			swapOrigins.y = centerPiece->getPositionY();
		}
		else
		{
			swapOrigins.z = centerPiece->getPositionX();
			swapOrigins.w = centerPiece->getPositionY();
		}

        if (!isCenterValid)
            swapPieces[i * swapIndexes.h() + 4] = NULL;
    }
    
    TileSwapper* tileSwapper = TileSwapper::getTileSwapper(swapOrigins, swapPieces);
	tileSwapper->retain();
	tileSwapper->setListener(this);
#if DEBUG
    std::vector<TileSwapper*>::iterator findIt = std::find(mAnimatingTileSwappers.begin(), mAnimatingTileSwappers.end(), tileSwapper);
    CCAssert(findIt == mAnimatingTileSwappers.end(), "PuzzleBoard: duplicate tile swapper found.");
#endif
    mAnimatingTileSwappers.push_back(tileSwapper);
    mCanvas[kLowerCanvas]->addChild(tileSwapper);
	mScene->addToJuggler(tileSwapper);
    mScene->playSound("tile-swap");
}

void PuzzleBoard::puzzleConveyorBeltWillMove(Coord moveDir, int wrapIndex, const int1d& tileIndexes, int numTiles)
{
	if (mPuzzle == NULL || numTiles <= 0 || wrapIndex < 0 || wrapIndex >= numTiles)
        return;

	std::vector<TilePiece*> beltTiles;
	beltTiles.resize(numTiles, NULL);
    for (int i = 0; i < numTiles; ++i)
	{
		beltTiles[i] = mTilePieces[tileIndexes.at(i)];

		// Mark end tiles as edge
		if (moveDir.x != 0 && (i == 0 || i == numTiles - 1))
		{
			int endIndex = tileIndexes.at(i) - mPuzzle->getNumColumns();
			if (mPuzzle->isValidIndex(endIndex))
				mTilePieces[endIndex]->setAestheticState(TilePiece::EDGE);
		}
	}

    CCRect boardBounds = getGlobalBoardClippingBounds();
    
    TileConveyorBelt* conveyorBelt = TileConveyorBelt::getTileConveyorBelt(boardBounds, moveDir, wrapIndex, beltTiles);
	conveyorBelt->retain();
	conveyorBelt->setListener(this);
	conveyorBelt->setDecorator(this);
    conveyorBelt->setEdgeStatus(getEdgeStatus(tileIndexes.at(0)));
#if DEBUG
    std::vector<TileConveyorBelt*>::iterator findIt = std::find(mAnimatingConveyorBelts.begin(), mAnimatingConveyorBelts.end(), conveyorBelt);
    CCAssert(findIt == mAnimatingConveyorBelts.end(), "PuzzleBoard: duplicate conveyor belt found.");
#endif
    mAnimatingConveyorBelts.push_back(conveyorBelt);
	mScene->addToJuggler(conveyorBelt);
    
    if (moveDir.x != 0)
        conveyorBelt->animate((mPuzzle->getNumColumns() / 10.0f) * 1.15f);
    else
        conveyorBelt->animate((mPuzzle->getNumRows() / 8.0f) * 0.9f * 1.15f);

	int zOrder = 1;
	if (mPuzzle->isRotating() && moveDir.x != 0)
	{
		const int2d& rotationIndexes = mPuzzle->getRotationIndexes();
		//int2d rotationIndexes = mPuzzle->getRotationIndexes();
		if (mPuzzle->rowForIndex(rotationIndexes.at(0, 0)) > mPuzzle->rowForIndex(tileIndexes.at(0)))
			zOrder = 0;
	}

	mCanvas[kUpperCanvas]->addChild(conveyorBelt, zOrder);

    // Add players that are positioned on the conveyor belt tiles.
	CCArray* players = mPuzzle->getPlayers();
	if (players)
	{
		CCObject* obj;
		CCARRAY_FOREACH(players, obj)
		{
			Player* player = static_cast<Player*>(obj);
			if (player)
			{
				int playerIndex = mPuzzle->pos2Index(player->getPosition());
				for (int i = 0; i < numTiles; ++i)
				{
					if (playerIndex == tileIndexes.at(i))
					{
						PlayerPiece* playerPiece = addPlayerToEffect(player);
						if (playerPiece)
							conveyorBelt->addPlayerPiece(i, playerPiece);
					}
				}
			}
		}
	}
    
    dispatchEvent(EV_TYPE_BG_EXPOSURE_WILL_BEGIN(), this);
}

void PuzzleBoard::puzzleWasSolved(int tileIndex)
{
	if (tileIndex >= 0 && tileIndex < (int)mTilePieces.size())
    {
		PlayfieldController* scene = dynamic_cast<PlayfieldController*>(mScene);
		if (scene)
		{
			TilePiece* tilePiece = mTilePieces[tileIndex];
			CCPoint tilePos = tilePiece->convertToWorldSpace(ccp(tilePiece->boundingBox().size.width / 2, tilePiece->boundingBox().size.height / 2));
			scene->getPuzzleController()->displaySolvedAnimation(tilePos, ccp(mScene->getViewWidth() / 2, mScene->getViewHeight() / 2));
		}
    }
}

PlayerPiece* PuzzleBoard::addPlayerToEffect(Player* player)
{
	if (player == NULL)
		return NULL;

	PlayerPiece* playerPiece = PlayerPiece::getPlayerPiece(player);
	playerPiece->setData(NULL);
	playerPiece->setPositionAestheticOnly(CCPointZero);
	playerPiece->setScaleX(1.0f / kTileScaleX);

	PlayerPiece* realPlayerPiece = static_cast<PlayerPiece*>(mPlayerPieces->objectForKey(player->getID()));
	if (realPlayerPiece)
	{
		playerPiece->syncWithPlayerPiece(realPlayerPiece);
		realPlayerPiece->setVisible(false);
	}
	
	if (player->getType() == Player::MIRRORED_MATE)
	{
		MirroredPlayer* mirroredPlayer = dynamic_cast<MirroredPlayer*>(player);
		if (mirroredPlayer && mirroredPlayer->getNumMovesRemaining() == 0)
			playerPiece->transitionOut(1.0f); // Will not trigger double hibernate because event isn't hooked up.
	}

	return playerPiece;
}

void PuzzleBoard::onEvent(int evType, void* evData)
{
	if (evType == ByteTweener::EV_TYPE_BYTE_TWEENER_CHANGED())
	{
		ByteTweener* tweener = static_cast<ByteTweener*>(evData);
		if (tweener)
		{
			GLubyte tweenedValue = tweener->getTweenedValue();
			if (tweener->getTag() == kByteTweenerPathTag)
			{
				if (mPathIndexes)
				{
					for (int i = 0; i < mNumPathIndexes; ++i)
						mTilePieces[mPathIndexes[i]]->setPathOpacity(tweenedValue);
				}
			}
            else if (tweener->getTag() == kByteTweenerIqTag)
            {
                if (mIqTag)
                    mIqTag->setOpacity(tweenedValue);
                if (mIqLabel[kIQLabelIndexLGE])
                    mIqLabel[kIQLabelIndexLGE]->setOpacity(tweenedValue);
            }
		}
	}
	else if (evType == ByteTweener::EV_TYPE_BYTE_TWEENER_COMPLETED())
	{
		ByteTweener* tweener = static_cast<ByteTweener*>(evData);
		if (tweener)
		{
			if (tweener->getTag() == kByteTweenerPathTag)
			{
				disablePath();
			}
			else if (tweener->getTag() == kByteTweenerTeleportTag)
			{
				tweener->reverse();
			}
			else if (tweener->getTag() == kByteTweenerKeyTag)
			{
				tweener->reset(0, 255, 6.0f);
			}
		}
	}
	else if (evType == PlayerPiece::EV_TYPE_DID_TRANSITION_OUT())
	{
		PlayerPiece* playerPiece = static_cast<PlayerPiece*>(evData);
		if (playerPiece)
		{
			playerPiece->removeEventListener(PlayerPiece::EV_TYPE_DID_TRANSITION_OUT(), this);
            playerPiece->returnToPool();
			playerPiece->autorelease();
		}
	}
	else if (evType == TileConveyorBelt::EV_TYPE_ANIMATION_COMPLETED())
	{
		tileConveyorBeltCompleted(static_cast<TileConveyorBelt*>(evData));
	}
	else if (evType == TileRotator::EV_TYPE_ANIMATION_COMPLETED())
	{
		tileRotatorCompleted(static_cast<TileRotator*>(evData));
	}
	else if (evType == TileSwapper::EV_TYPE_ANIMATION_COMPLETED())
	{
		tileSwapperCompleted(static_cast<TileSwapper*>(evData));
	}
	else if (evType == Puzzle::EV_TYPE_PLAYER_ADDED())
	{
		Player* player = static_cast<Player*>(evData);
		if (player)
			addPlayerPiece(player);
	}
	else if (evType == Puzzle::EV_TYPE_PLAYER_REMOVED())
	{
		Player* player = static_cast<Player*>(evData);
		if (player)
			removePlayerPiece(player, mPuzzle && mPuzzle->isResetting() ? 0.25f : 1.0f);
	}
	else if (evType == FloatTweener::EV_TYPE_FLOAT_TWEENER_CHANGED())
	{
		FloatTweener* tweener = static_cast<FloatTweener*>(evData);
		if (tweener)
		{
			int tileIndex = tweener->getTag() - (tweener->getTag() >= kTransitionOutTag ? kTransitionOutTag : kTransitionInTag);
			CCAssert(tileIndex < (int)mTilePieces.size(), "PuzzleBoard - bad transition index.");
			mTilePieces[tileIndex]->setPositionX(tweener->getTweenedValue());
		}
	}
	else if (evType == FloatTweener::EV_TYPE_FLOAT_TWEENER_COMPLETED())
	{
		FloatTweener* tweener = static_cast<FloatTweener*>(evData);
		if (tweener)
		{
			int tileIndex = tweener->getTag() - (tweener->getTag() > kTransitionOutTag ? kTransitionOutTag : kTransitionInTag);
			if (tileIndex == (int)mTilePieces.size() - 1)
			{
				setTransitioning(false);
                
                for(std::vector<TilePiece*>::iterator it = mTilePieces.begin(); it != mTilePieces.end(); ++it)
                    (*it)->setAestheticState(TilePiece::NORMAL);

				if (tweener->getTag() >= kTransitionOutTag)
				{
					setVisible(false);
					dispatchEvent(EV_TYPE_DID_TRANSITION_OUT(), this);
				}
				else
					dispatchEvent(EV_TYPE_DID_TRANSITION_IN(), this);
			}
		}
	}
    else if (evType == Puzzle::EV_TYPE_FORCE_COMPLETE_ALL_MECHANICS())
    {
        forceCompleteActiveAnimations();
    }
}

void PuzzleBoard::tileConveyorBeltCompleted(TileConveyorBelt* conveyorBelt)
{
	if (conveyorBelt == NULL)
		return;
    std::vector<TileConveyorBelt*>::iterator findIt = std::find(mAnimatingConveyorBelts.begin(), mAnimatingConveyorBelts.end(), conveyorBelt);
    CCAssert(findIt != mAnimatingConveyorBelts.end(), "PuzzleBoard: invalid conveyor belt completed.");
    if (findIt == mAnimatingConveyorBelts.end())
        return;
    
    mAnimatingConveyorBelts.erase(std::remove(mAnimatingConveyorBelts.begin(), mAnimatingConveyorBelts.end(), conveyorBelt));
	conveyorBelt->setListener(NULL);
    conveyorBelt->returnToPool();
	CC_SAFE_RELEASE_NULL(conveyorBelt);

    if (mPuzzle && mPlayerPieces)
    {
		mPuzzle->applyConveyorBelt();
        CCDictElement* kvp;
		CCDICT_FOREACH(mPlayerPieces, kvp)
		{
			Player* player = mPuzzle->getPlayerForID((int)kvp->getIntKey());
			PlayerPiece* playerPiece = static_cast<PlayerPiece*>(kvp->getObject());
			if (playerPiece && player && !mPuzzle->isPlayerOccupied(player))
				playerPiece->setVisible(true);
		}

		int1d* tileIndexes = mPuzzle->getConveyorBeltIndexes();
		int numRows = mPuzzle->getNumRows(), numIndexes = mPuzzle->getConveyorBeltIndexCount();
		Coord conveyorBeltDir = mPuzzle->getConveyorBeltDir();
		for (int i = 0; i < numIndexes; ++i)
		{
			int tileIndex = tileIndexes->at(i);
			TilePiece* tilePiece = mTilePieces[tileIndex];
			CCSize tileDims = tilePiece->tileDimensions();
			tilePiece->setPosition(ccp(mPuzzle->columnForIndex(tileIndex) * tileDims.width, (numRows - (mPuzzle->rowForIndex(tileIndex) + 1)) * tileDims.height));
			tilePiece->setAestheticState(TilePiece::NORMAL);
            tilePiece->setVisible(true);

			// Unmark end tiles as edge
			if (conveyorBeltDir.x != 0 && (i == 0 || i == numIndexes - 1))
			{
				int endIndex = tileIndexes->at(i) - mPuzzle->getNumColumns();
				if (mPuzzle->isValidIndex(endIndex))
					mTilePieces[endIndex]->setAestheticState(TilePiece::NORMAL);
			}
		}
    }

	refreshPlayerZValues();
    dispatchEvent(EV_TYPE_BG_EXPOSURE_DID_END(), this);
}

void PuzzleBoard::tileRotatorCompleted(TileRotator* rotator)
{
	if (rotator == NULL)
		return;
    std::vector<TileRotator*>::iterator findIt = std::find(mAnimatingRotators.begin(), mAnimatingRotators.end(), rotator);
    CCAssert(findIt != mAnimatingRotators.end(), "PuzzleBoard: invalid rotator completed.");
    if (findIt == mAnimatingRotators.end())
        return;
    
    mAnimatingRotators.erase(std::remove(mAnimatingRotators.begin(), mAnimatingRotators.end(), rotator));
	rotator->setListener(NULL);
	rotator->returnToPool();
	CC_SAFE_RELEASE_NULL(rotator);

	if (mPuzzle && mPlayerPieces)
	{
		mPuzzle->applyRotation();
		CCDictElement* kvp;
		CCDICT_FOREACH(mPlayerPieces, kvp)
		{
			Player* player = mPuzzle->getPlayerForID((int)kvp->getIntKey());
			PlayerPiece* playerPiece = static_cast<PlayerPiece*>(kvp->getObject());
			if (playerPiece && player && !mPuzzle->isPlayerOccupied(player))
				playerPiece->setVisible(true);
		}

		const int2d& tileIndexes = mPuzzle->getRotationIndexes();
		for (int i = 0; i < 8; ++i)
			mTilePieces[tileIndexes.at(0, i+1)]->setVisible(true);

		// Revert presentation of relevant tiles in the row above which were marked as edge tiles while the animation was active
		for (int i = 0; i < 3; ++i)
		{
			//int edgeIndex = tileIndexes.at(0, kTileRotatorEdgeIndexes[i]) - (i == 0 ? 0 : mPuzzle->getNumColumns());
			int edgeIndex = tileIndexes.at(0, kTileRotatorEdgeIndexes[i]) - mPuzzle->getNumColumns();
			if (mPuzzle->isValidIndex(edgeIndex))
				mTilePieces[edgeIndex]->setAestheticState(TilePiece::NORMAL);
		}

		mTilePieces[tileIndexes.at(0, 0)]->setVisible(true);
	}

	refreshPlayerZValues();
    dispatchEvent(EV_TYPE_BG_EXPOSURE_DID_END(), this);
}

void PuzzleBoard::tileSwapperCompleted(TileSwapper* swapper)
{
	if (swapper == NULL)
		return;
    
    std::vector<TileSwapper*>::iterator findIt = std::find(mAnimatingTileSwappers.begin(), mAnimatingTileSwappers.end(), swapper);
    CCAssert(findIt != mAnimatingTileSwappers.end(), "PuzzleBoard: invalid tile swapper completed.");
    if (findIt == mAnimatingTileSwappers.end())
        return;
    
    mAnimatingTileSwappers.erase(std::remove(mAnimatingTileSwappers.begin(), mAnimatingTileSwappers.end(), swapper));
	swapper->setListener(NULL);
	swapper->returnToPool();
	CC_SAFE_RELEASE_NULL(swapper);

	if (mPuzzle)
		mPuzzle->applyTileSwap();
}

void PuzzleBoard::forceCompleteActiveAnimations(void)
{
    for (int i = (int)mAnimatingConveyorBelts.size() - 1; i >= 0; --i)
    {
        TileConveyorBelt* conveyorBelt = mAnimatingConveyorBelts[i];
        if (conveyorBelt && !conveyorBelt->isComplete())
        {
            conveyorBelt->stopAnimating();
            tileConveyorBeltCompleted(conveyorBelt);
        }
    }
    mAnimatingConveyorBelts.clear();
    
    for (int i = (int)mAnimatingRotators.size() - 1; i >= 0; --i)
    {
        TileRotator* rotator = mAnimatingRotators[i];
        if (rotator && !rotator->isComplete())
        {
            rotator->stopAnimating();
            tileRotatorCompleted(rotator);
        }
    }
    mAnimatingRotators.clear();
    
    for (int i = (int)mAnimatingTileSwappers.size() - 1; i >= 0; --i)
    {
        TileSwapper* tileSwapper = mAnimatingTileSwappers[i];
        if (tileSwapper && !tileSwapper->isComplete())
        {
            tileSwapper->stopAnimating();
            tileSwapperCompleted(tileSwapper);
        }
    }
    mAnimatingTileSwappers.clear();
}

void PuzzleBoard::reset(bool playSound)
{
    forceCompleteActiveAnimations();
    
	if (mPuzzle && !isTransitioning())
    {
        mShieldManager->withdrawAll(false);
		mPuzzle->reset();

        if (mPlayerPieces)
        {
			CCDictElement* kvp;
			CCDICT_FOREACH(mPlayerPieces, kvp)
			{
				PlayerPiece* playerPiece = static_cast<PlayerPiece*>(kvp->getObject());
				if (playerPiece)
					playerPiece->reset();
			}
        }

        if (playSound)
			mScene->playSound("reset");
    }
    
	mAddQueue->removeAllObjects();
	mRemoveQueue->removeAllObjects();
    hidePathNotFound();
}

void PuzzleBoard::advanceTime(float dt)
{
	mIsLocked = true;
	CCDictElement* kvp;
	CCDICT_FOREACH(mPlayerPieces, kvp)
	{
		PlayerPiece* playerPiece = static_cast<PlayerPiece*>(kvp->getObject());
		if (playerPiece->isAdvanceable())
			playerPiece->advanceTime(dt);
	}
    mIsLocked = false;

	CCObject* obj;
	CCARRAY_FOREACH(mAddQueue, obj)
	{
		Player* player = static_cast<Player*>(obj);
		if (player)
			addPlayerPiece(player);
	}
	CCARRAY_FOREACH(mRemoveQueue, obj)
	{
		Player* player = static_cast<Player*>(obj);
		if (player)
			removePlayerPiece(player);
	}
	mAddQueue->removeAllObjects();
	mRemoveQueue->removeAllObjects();

	mShieldManager->advanceTime(dt);
    mColorArrowClip->advanceTime(dt);
	
	if (isTransitioning())
	{
		for (int i = 0; i < (int)mTransitions.size(); ++i)
			mTransitions[i]->advanceTime(dt);
	}
    
    if (mIqTweener && !mIqTweener->isComplete())
        mIqTweener->advanceTime(dt);
    
    if (mPathNotFound && mPathNotFound->isVisible())
    {
        mPathNotFoundTimer += dt;
        
        if (mPathNotFoundTimer < kPathNotFoundDuration)
            mPathNotFound->setOpacity(255 - 255 * CMTransitions::linear(mPathNotFoundTimer / kPathNotFoundDuration));
        else
            hidePathNotFound();
    }
}

void PuzzleBoard::postAdvanceTime(float dt)
{
	mTeleportTweener->advanceTime(dt);
	mKeyTweener->advanceTime(dt);
}


