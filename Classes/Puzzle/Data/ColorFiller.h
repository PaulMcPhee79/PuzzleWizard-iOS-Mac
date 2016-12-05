#ifndef __COLOR_FILLER_H__
#define __COLOR_FILLER_H__

#include "cocos2d.h"
class Puzzle;
class PuzzleTile;
USING_NS_CC;

class ColorFiller : public CCObject
{
public:
	ColorFiller(void);
	virtual ~ColorFiller(void);

	int fill(Puzzle* puzzle, PuzzleTile* originTile, uint colorKey);

private:
    static const int kNumAdjIndexes = 4;
	static const int* kColorFillIndexOffsets;
	static int kNodeCacheSize;

	ColorFiller(const ColorFiller& other);
	ColorFiller& operator=(const ColorFiller& rhs);

	uint nextGUID(void);
	void fillColorFillCache(void);
	int fillRoot(PuzzleTile* tile, uint colorKey);
	int fillNode(int index, PuzzleTile* tile, uint colorKey, int fillDepth, uint guid);

	uint mGUID;
	size_t mNodeIter;
	int mRootNodeIndexes[kNumAdjIndexes];
	std::vector<int*> mNodeIndexes;

	// On loan from client for duration of fill call.
	PuzzleTile* mTiles;
	Puzzle* mPuzzle;
};
#endif // __COLOR_FILLER_H__
