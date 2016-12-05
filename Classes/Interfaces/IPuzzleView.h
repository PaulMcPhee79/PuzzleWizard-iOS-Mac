#ifndef __IPUZZLE_VIEW_H__
#define __IPUZZLE_VIEW_H__

#include <Utils/CMTypes.h>
class Player;

class IPuzzleView
{
public:
    virtual ~IPuzzleView(void) { }
	virtual void puzzleSoundShouldPlay(const char* soundName) = 0;
	virtual void puzzlePlayerWillMove(Player* player) = 0;
	virtual void puzzlePlayerDidMove(Player* player) = 0;
    virtual void puzzleShieldDidDeploy(int tileIndex) = 0;
    virtual void puzzleShieldWasWithdrawn(int tileIndex) = 0;
    virtual void puzzleTilesShouldRotate(const int2d& tileIndexes) = 0;
    virtual void puzzleTileSwapWillBegin(const int2d& swapIndexes, bool isCenterValid) = 0;
	virtual void puzzleConveyorBeltWillMove(Coord moveDir, int wrapIndex, const int1d& tileIndexes, int numTiles) = 0;
    virtual void puzzleWasSolved(int tileIndex) = 0;
};
#endif // __IPUZZLE_VIEW_H__
