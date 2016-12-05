#ifndef __PATH_FINDER_H__
#define __PATH_FINDER_H__

#include "cocos2d.h"
#include <Interfaces/ISearchProvider.h>
#include <Utils/Astar.h>
#include <Puzzle/Inputs/SearchNode.h>
#include <Utils/CMTypes.h>
class Puzzle;
class Player;
class PuzzleTile;
USING_NS_CC;

class PathFinder : public CCObject, public ISearchProvider
{
public:
	PathFinder(int maxSearchSize = 100);
	virtual ~PathFinder(void);

	virtual int getSuccessors(const Coord& tilePos, const Coord& parentPos, Coord* successors, int numAncestors);
	virtual const Coord& getDxy(void) const { return mDxy; }
	virtual int getSearchWeighting(const Coord& tilePos) const;
    virtual int getMaxPathLength() const { return mMaxSearchSize; }
	int getFoundPath(Coord* path, int maxLength);
	bool findPathForPlayer(Player* player, const Coord& from, const Coord& to, int maxIterations = -1);

	const Puzzle* getDataProvider(void) const { return mPuzzle; }
	void setDataProvider(Puzzle* value);
    bool isBusy(void) const;
    void enableSearchWeighting(bool enable);
    void cancelSearch(void);

	// Convenience pass-throughs to data provider
	int pos2Index(const Coord& pos) const;
	PuzzleTile* tileAtIndex(int index) const;

private:
	unsigned int mSearchState;
    int mMaxSearchSize;
	Coord mFrom;
	Coord mTo;
	Coord mDxy;
	Player* mPlayer;
	Puzzle* mPuzzle;
	AStarSearch<SearchNode> mAstar;
};
#endif // __PATH_FINDER_H__
