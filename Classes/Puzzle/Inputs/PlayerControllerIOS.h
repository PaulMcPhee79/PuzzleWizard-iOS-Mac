
#ifndef __PLAYER_CONTROLLER_IOS_H__
#define __PLAYER_CONTROLLER_IOS_H__

#include "cocos2d.h"
#include <Puzzle/Inputs/PlayerController.h>
class Player;
class PuzzleBoard;
class PathFinder;
USING_NS_CC;

class PlayerControllerIOS : public PlayerController
{
public:
	PlayerControllerIOS(PuzzleBoard* board);
	virtual ~PlayerControllerIOS(void);

	virtual void advanceTime(float dt);
	virtual void onEvent(int evType, void* evData);

	virtual void playerValueDidChange(uint code, int value);
	virtual void willBeginMoving(void);
    virtual void didFinishMoving(void);
    virtual void didIdle(void);

	virtual void reset(void);
	virtual void updateBoardBounds(void);
	virtual void setPlayer(Player* value);
	virtual const Coord* getPath(void) const { return mFoundPath + mFoundPathIndex; }
	virtual int getPathLength(void) const { return mFoundPathLen - mFoundPathIndex; }
	virtual void setPathFinder(PathFinder* value);

    virtual void puzzleTilesShouldRotate(const int2d& tileIndexes);
    virtual void puzzleTileSwapWillBegin(const int2d& swapIndexes, bool isCenterValid);
	virtual void puzzleConveyorBeltWillMove(Coord moveDir, int wrapIndex, const int1d& tileIndexes, int numTiles);
	virtual void puzzleWasSolved(int tileIndex);

private:
	static const int kMaxFoundPathLen = 100;
    
    void tryMovePlayer(void);

	vec2 mBoardOffset;
	vec2 mTileDimensions;
	vec2 mBoardDimensions;

	int mFoundPathIndex;
	int mFoundPathLen;
	Coord* mFoundPath;
	PathFinder* mPathFinder;
	PuzzleBoard* mBoard;
};
#endif // __PLAYER_CONTROLLER_IOS_H__
