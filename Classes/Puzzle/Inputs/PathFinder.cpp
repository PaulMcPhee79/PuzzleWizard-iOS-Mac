
//#define PATH_FIND_AVOID_SPECIAL_TILES

#include "PathFinder.h"
#include <Puzzle/Data/Puzzle.h>
#include <Puzzle/Data/PuzzleTile.h>
#include <Puzzle/Data/Player.h>
#include <Puzzle/View/Playfield/TilePiece.h>
USING_NS_CC;

PathFinder::PathFinder(int maxSearchSize)
	: mPuzzle(NULL), mPlayer(NULL), mAstar(maxSearchSize), mSearchState(AStarSearch<SearchNode>::SEARCH_STATE_NOT_INITIALISED), mMaxSearchSize(maxSearchSize)
{
	
}

PathFinder::~PathFinder(void)
{
	CC_SAFE_RELEASE_NULL(mPuzzle);
	mAstar.FreeSolutionNodes();
	mAstar.EnsureMemoryFreed();
}

bool PathFinder::isBusy(void) const
{
    bool busyState = mSearchState == AStarSearch<SearchNode>::SEARCH_STATE_SEARCHING;
    if (mPuzzle)
		busyState = busyState || mPuzzle->isConveyorBeltActive() || mPuzzle->isRotating() || mPuzzle->isTileSwapping();
    return busyState;
}

void PathFinder::enableSearchWeighting(bool enable)
{
    if (mPuzzle)
        mPuzzle->enableSearchWeighting(enable);
}

void PathFinder::cancelSearch(void)
{
    if (mSearchState == AStarSearch<SearchNode>::SEARCH_STATE_SEARCHING)
    {
        mAstar.CancelSearch();
        mAstar.SearchStep();
    }
}

void PathFinder::setDataProvider(Puzzle* value)
{
	if (mPuzzle == value)
		return;
	CC_SAFE_RETAIN(value);
	CC_SAFE_RELEASE(mPuzzle);
    mPuzzle = value;
}

int PathFinder::getSearchWeighting(const Coord& tilePos) const
{
	if (mPuzzle == NULL || mFrom == tilePos || mTo == tilePos)
		return 0;

	int index = mPuzzle->pos2Index(tilePos), weighting = 0;
	PuzzleTile* tile = mPuzzle->tileAtIndex(index);
	if (tile)
	{
		weighting = mPuzzle->getSearchWeighting(index);
		//if (weighting)
		//	CCLog("Tile::getFuctionKey: %u Weighting: %d", tile ? tile->getFunctionKey() : 0, weighting);
	}

	return weighting;
}

int PathFinder::getSuccessors(const Coord& tilePos, const Coord& parentPos, Coord* successors, int numAncestors)
{
	if (mPuzzle == NULL || successors == NULL)
		return 0;

	int originIndex = mPuzzle->pos2Index(tilePos);
	PuzzleTile* originTile = mPuzzle->tileAtIndex(originIndex);
	if (originTile == NULL)
		return 0;

	Coord pos = cmc(-1, -1);
	bool mColorMagicActive = false;
	if (mPlayer && mPlayer->isColorMagicActive())
		mColorMagicActive = mPlayer->getNumColorMagicMoves() - (mPlayer->isMoving() ? 1 : 0) > numAncestors;
	
	int numSuccessors = 0, numColumns = mPuzzle->getNumColumns(), numRows = mPuzzle->getNumRows();
	for (int i = 0; i < 4; ++i)
	{
		pos.x = pos.y = -1;

		switch (i)
		{
			case 0: // North
				{
					pos.x = tilePos.x;
					pos.y = tilePos.y - 1;
				}
				break;
			case 1: // East
				{
					pos.x = tilePos.x + 1;
					pos.y = tilePos.y;
				}
				break;
			case 2: // South
				{
					pos.x = tilePos.x;
					pos.y = tilePos.y + 1;
				}
				break;
			case 3: // West
				{
					pos.x = tilePos.x - 1;
					pos.y = tilePos.y;
				}
				break;
		}

		if (pos.x >= 0 && pos.y >= 0 && pos.x < numColumns && pos.y < numRows && pos != parentPos)
		{
			int tileIndex = mPuzzle->pos2Index(pos);
			PuzzleTile* tile = mPuzzle->tileAtIndex(tileIndex);
            
#ifdef PATH_FIND_AVOID_SPECIAL_TILES
			if (tile && (tile->getFunctionKey() == 0 || pos == mTo) && (tile->getColorKey() == originTile->getColorKey()
				|| tile->getColorKey() == TilePiece::kColorKeyWhite
				|| originTile->getColorKey() == TilePiece::kColorKeyWhite))
#else
			if (tile && (mColorMagicActive || (tile->getColorKey() == originTile->getColorKey()
				|| tile->getColorKey() == TilePiece::kColorKeyWhite
				|| originTile->getColorKey() == TilePiece::kColorKeyWhite)))
#endif
			{
				successors[numSuccessors++] = pos;
			}
		}
	}

	return numSuccessors;
}

int PathFinder::getFoundPath(Coord* path, int maxLength)
{
	if (mPuzzle == NULL || mSearchState != AStarSearch<SearchNode>::SEARCH_STATE_SUCCEEDED)
		return 0;

	int i = 0;
	SearchNode* node = mAstar.GetSolutionStart();
	
	for(; i < maxLength; ++i)
	{
		node = mAstar.GetSolutionNext();
		if (!node)
			break;

		path[i] = cmc(node->x, node->y);
	}

	return i;
}

bool PathFinder::findPathForPlayer(Player* player, const Coord& from, const Coord& to, int maxIterations)
{
	if (mPuzzle == NULL)
		return true;

	CCAssert(mPlayer == NULL, "PathFinder::findPathForPlayer did not discard last player.");
	mPlayer = player;

	if (mSearchState != AStarSearch<SearchNode>::SEARCH_STATE_SEARCHING)
	{
		// Begin new search
		mAstar.FreeSolutionNodes();
		mFrom = cmc(from.x, from.y);
		mTo = cmc(to.x, to.y);
		mDxy = cmc(from.x - to.x, from.y - to.y);
        SearchNode start = SearchNode(from.x, from.y, this), goal = SearchNode(to.x, to.y, this);
		mAstar.SetStartAndGoalStates(start, goal);
	}

	int i = 0;
	for (; i < maxIterations || maxIterations == -1; ++i)
	{
		mSearchState = mAstar.SearchStep();
		if (mSearchState != AStarSearch<SearchNode>::SEARCH_STATE_SEARCHING)
			break;
	}

	mPlayer = NULL;

	if (mSearchState == AStarSearch<SearchNode>::SEARCH_STATE_SEARCHING)
	{
		CCLog("PathFinder: Path NOT found after %d steps. Continuing search next frame...", i);
		return false;
	}
	else
	{
		if (mSearchState == AStarSearch<SearchNode>::SEARCH_STATE_SUCCEEDED)
			CCLog("PathFinder: Path found after %d steps.", i);
		else
			CCLog("PathFinder: Failed to find path with state %u.", mSearchState);

		return true;
	}
}

int PathFinder::pos2Index(const Coord& pos) const
{
	return mPuzzle ? mPuzzle->pos2Index(pos) : -1;
}

PuzzleTile* PathFinder::tileAtIndex(int index) const
{
	return mPuzzle ? mPuzzle->tileAtIndex(index) : NULL;
}
