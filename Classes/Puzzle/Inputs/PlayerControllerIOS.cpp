
#include "PlayerControllerIOS.h"
#include <Puzzle/Inputs/TouchPad.h>
#include <Puzzle/Inputs/PathFinder.h>
#include <Puzzle/Data/Player.h>
#include <Puzzle/View/Playfield/PuzzleBoard.h>
#include <Puzzle/Data/PuzzleTile.h>
#include <Managers/TouchTypes.h>
USING_NS_CC;

PlayerControllerIOS::PlayerControllerIOS(PuzzleBoard* board)
	:
	mBoardOffset(0, 0),
	mTileDimensions(72, 72),
    mBoardDimensions(MODE_8x6 ? 8 * 72 : 10 * 72, MODE_8x6 ? 6 * 72 : 8 * 72),
	mFoundPathIndex(0),
	mFoundPathLen(0),
	mFoundPath(NULL),
	mPathFinder(NULL),
	mBoard(board)
{
	CCAssert(board != NULL, "PlayerControllerIOS requires a non-NULL PuzzleBoard.");
	board->retain();
	updateBoardBounds();
}

PlayerControllerIOS::~PlayerControllerIOS(void)
{
	setPathFinder(NULL);
	setPlayer(NULL);
	CC_SAFE_RELEASE_NULL(mBoard);

	if (mFoundPath)
	{
		delete[] mFoundPath, mFoundPath = NULL;
	}
}

void PlayerControllerIOS::updateBoardBounds(void)
{
	mTileDimensions = mBoard->getTileDimensions();
	mBoardDimensions = mBoard->getBoardDimensions();
	mBoardOffset = cmv2(mBoard->getPositionX() - mTileDimensions.x / 2, mBoard->getPositionY() - mTileDimensions.y / 2);
}

void PlayerControllerIOS::setPlayer(Player* value)
{
	if (mPlayer == value)
		return;
	if (mPlayer)
		mPlayer->deregisterView(this);
    
    PlayerController::setPlayer(value);

	if (mPlayer)
		mPlayer->registerView(this);
}

void PlayerControllerIOS::setPathFinder(PathFinder* value)
{
	if (mPathFinder == value)
		return;
	if (value && mFoundPath == NULL)
		mFoundPath = new Coord[kMaxFoundPathLen];
	CC_SAFE_RETAIN(value);
	CC_SAFE_RELEASE(mPathFinder);
    mPathFinder = value;
	mFoundPathIndex = mFoundPathLen = 0;
}

void PlayerControllerIOS::reset(void)
{
    PlayerController::reset();
	mFoundPathIndex = mFoundPathLen = 0;
}

void PlayerControllerIOS::playerValueDidChange(uint code, int value)
{
	switch (code)
    {
		case Player::kValueProperty:
            break;
        case Player::kValueColorMagic:
			if (value == Player::maxColorMagicMoves() && mFoundPathIndex != 0 && mFoundPathLen != 0)
				dispatchEvent(EV_TYPE_PLAYER_WILL_MOVE());
            break;
        case Player::kValueMirroredMate:
            break;
		case Player::kValueTeleported:
			reset();

			if (mPlayer)
				mPlayer->setQueuedMove(cmc(0, 0));
			break;
    }
}

void PlayerControllerIOS::willBeginMoving(void)
{
	if (mFoundPathIndex < mFoundPathLen)
		++mFoundPathIndex;
}

void PlayerControllerIOS::didFinishMoving(void)
{
	tryMovePlayer();
}

void PlayerControllerIOS::didIdle(void)
{
    reset();
}

void PlayerControllerIOS::puzzleTilesShouldRotate(const int2d& tileIndexes)
{
	mFoundPathIndex = mFoundPathLen = 0;
	dispatchEvent(EV_TYPE_PLAYER_STOPPED_SHORT());
}

void PlayerControllerIOS::puzzleTileSwapWillBegin(const int2d& swapIndexes, bool isCenterValid)
{
	mFoundPathIndex = mFoundPathLen = 0;
	dispatchEvent(EV_TYPE_PLAYER_STOPPED_SHORT());
}

void PlayerControllerIOS::puzzleConveyorBeltWillMove(Coord moveDir, int wrapIndex, const int1d& tileIndexes, int numTiles)
{
	mFoundPathIndex = mFoundPathLen = 0;
	dispatchEvent(EV_TYPE_PLAYER_STOPPED_SHORT());
}

void PlayerControllerIOS::puzzleWasSolved(int tileIndex)
{
	mFoundPathIndex = mFoundPathLen = 0;
	dispatchEvent(EV_TYPE_PLAYER_STOPPED_SHORT());
}

void PlayerControllerIOS::onEvent(int evType, void* evData)
{
	if (!isEnabled())
		return;

	if (evType == TouchPad::EV_TYPE_TOUCH_BEGAN())
	{
		CMTouches::TouchNotice* touchNotice = (CMTouches::TouchNotice*)evData;
		if (touchNotice == NULL)
			return;
        
		if (mPlayer && mPathFinder && !mPathFinder->isBusy())
		{
			CCPoint touchPos = mBoard->convertToNodeSpace(touchNotice->pos);
			touchPos.x += mTileDimensions.x / 2; touchPos.y += mTileDimensions.y / 2;

			// Convert touch coords to tile index
            vec2 destVec = cmv2(touchPos.x / mTileDimensions.x, (mBoardDimensions.y - touchPos.y) / mTileDimensions.y);
            if (destVec.x >= 0 && destVec.y >= 0)
            {
                Coord dest = cmc((int)destVec.x, (int)destVec.y);
                int index = mPathFinder->pos2Index(dest);
                PuzzleTile* tile = mPathFinder->tileAtIndex(index);
                if (tile)
                {
                	if (isLicenseLocked())
                	{
                		dispatchEvent(EV_TYPE_LICENSE_LOCK_DID_TRIGGER(), this);
                		return;
                	}

                    touchNotice->retainFocus(this);
                    
                    // Find a path and store it
                    bool pathFound = mPathFinder->findPathForPlayer(mPlayer, mPlayer->isMoving() ? mPlayer->getIsMovingTo() : mPlayer->getPosition(), dest);
                    if (pathFound)
                    {
                        mFoundPathLen = mPathFinder->getFoundPath(mFoundPath, kMaxFoundPathLen);
                        mFoundPathIndex = 0;
                        
                        if (mFoundPathLen == 0 && mPlayer->isColorMagicActive())
                        {
                            mPathFinder->enableSearchWeighting(false);
                            pathFound = mPathFinder->findPathForPlayer(mPlayer, mPlayer->isMoving() ? mPlayer->getIsMovingTo() : mPlayer->getPosition(), dest);
                            mPathFinder->enableSearchWeighting(true);
                            
                            if (pathFound)
                            {
                                mFoundPathLen = mPathFinder->getFoundPath(mFoundPath, kMaxFoundPathLen);
                                mFoundPathIndex = 0;
                            }
                        }
                        
                        if (pathFound)
                        {
                            if (mFoundPathLen > 0)
                                dispatchEvent(EV_TYPE_PLAYER_WILL_MOVE());
                            else
                                dispatchEvent(EV_TYPE_PATH_NOT_FOUND(), &index);
                        }
                    }
                    
                    if (!pathFound)
                        mPathFinder->cancelSearch();
                }
            }
		}
	}
}

void PlayerControllerIOS::tryMovePlayer(void)
{
    if (mPlayer && mFoundPath && mFoundPathIndex < mFoundPathLen)
	{
		if (!mPlayer->isMoving())
		{
			Coord currentPos = mPlayer->getPosition();
			mPlayer->setQueuedMove(cmc(mFoundPath[mFoundPathIndex].x - currentPos.x, mFoundPath[mFoundPathIndex].y - currentPos.y));
			//++mFoundPathIndex; // Done in PlayerControllerIOS::willBeginMoving
		}
	}
}

void PlayerControllerIOS::advanceTime(float dt)
{
	tryMovePlayer();
}
