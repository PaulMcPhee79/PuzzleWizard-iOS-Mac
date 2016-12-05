
#include "ColorFiller.h"
#include <Puzzle/Data/Puzzle.h>
#include <Puzzle/Data/PuzzleTile.h>
#include <Utils/DeviceDetails.h>
USING_NS_CC;

static const int kNodeCacheSize_8x6 = 48;
static const int kNodeCacheSize_10x8 = 80;

static const int kColorFillIndexOffsets_8x6[] = { 1, 8, 10, 17 };
static const int kColorFillIndexOffsets_10x8[] = { 1, 10, 12, 21 };

const int* ColorFiller::kColorFillIndexOffsets = NULL;
int ColorFiller::kNodeCacheSize = -1;

ColorFiller::ColorFiller(void)
	: mNodeIter(-1), mGUID(2), mPuzzle(NULL), mTiles(NULL) // GUID: 0: Invalid 1: Tile starting index
{
    if (kColorFillIndexOffsets == NULL)
    {
        kColorFillIndexOffsets = MODE_8x6 ? kColorFillIndexOffsets_8x6 : kColorFillIndexOffsets_10x8;
    }
    
    if (kNodeCacheSize == -1)
    {
        kNodeCacheSize = MODE_8x6 ? kNodeCacheSize_8x6 : kNodeCacheSize_10x8;
    }
    
	std::fill_n(mRootNodeIndexes, kNumAdjIndexes, -1);
	fillColorFillCache();
}

ColorFiller::~ColorFiller(void)
{
	for(size_t i = 0; i < mNodeIndexes.size(); ++i)
		delete [] mNodeIndexes[i];
	mNodeIndexes.clear();

	mPuzzle = NULL;
	mTiles = NULL;
}

uint ColorFiller::nextGUID(void)
{
	return mGUID++;
}

void ColorFiller::fillColorFillCache(void)
{
	for (size_t i = mNodeIndexes.size(); i < kNodeCacheSize; ++i)
	{
		int* arr = new int[kNumAdjIndexes];
		std::fill_n(arr, kNumAdjIndexes, -1);
		mNodeIndexes.push_back(arr);
	}
}

int ColorFiller::fill(Puzzle* puzzle, PuzzleTile* originTile, uint colorKey)
{
	int fillCount = 0;
	mPuzzle = puzzle;
	if (mPuzzle)
	{
		mGUID = 2;
		mTiles = mPuzzle->getTiles();
		fillCount = fillRoot(originTile, colorKey);
		mPuzzle = NULL; mTiles = NULL;
	}

	return fillCount;
}

int ColorFiller::fillRoot(PuzzleTile* tile, uint colorKey)
{
	int rootIndex = mPuzzle->indexForTile(tile), topLeftIndex = rootIndex - (mPuzzle->getNumColumns() + 1), numLiveNodes = 0;
    int* nodeIndexes = mRootNodeIndexes;

    for (int i = 0; i < kNumAdjIndexes; ++i)
    {
        nodeIndexes[i] = -1;

        int nodeIndex = topLeftIndex + kColorFillIndexOffsets[i];
        if (mPuzzle->isValidIndex(nodeIndex))
        {
            if (!mPuzzle->doesAdjacentPerpIndexWrap(rootIndex, nodeIndex))
            {
				PuzzleTile* nodeTile = &mTiles[nodeIndex];
                if (nodeTile->getColorKey() == colorKey && !nodeTile->isModified(PuzzleTile::kTMShielded))
                {
                    nodeIndexes[i] = nodeIndex;
                    ++numLiveNodes;
                }
            }
        }   
    }

	int numTiles = mPuzzle->getNumTiles();
	for (int i = 0; i < numTiles; ++i)
		mTiles[i].setGUID(1);

    int totalFills = fillNode(rootIndex, tile, colorKey, 1, nextGUID());
    if (totalFills > 0)
    {
        while (numLiveNodes != 0)
        {
            tile->setGUID(nextGUID());
            for (int i = 0; i < kNumAdjIndexes; ++i)
            {
                int nodeIndex = nodeIndexes[i];
                if (nodeIndex == -1)
                    continue;

                PuzzleTile* nodeTile = &mTiles[nodeIndex];
                int nodeFills = fillNode(nodeIndex, nodeTile, colorKey, 2, tile->getGUID());
                if (nodeFills == 0)
                {
                    nodeIndexes[i] = -1;
                    --numLiveNodes;
                }
                
                totalFills += nodeFills;
            }
        }
    }

	return totalFills;
}

int ColorFiller::fillNode(int index, PuzzleTile* tile, uint colorKey, int fillDepth, uint guid)
{
    int numFills = 0, numPreFills = 0, topLeftIndex = index - (mPuzzle->getNumColumns() + 1);
    int* subNodeIndexes = NULL;
    
	if (mNodeIter < mNodeIndexes.size())
        subNodeIndexes = mNodeIndexes[mNodeIter];
    else
    {
        subNodeIndexes = new int[kNumAdjIndexes];
		std::fill_n(subNodeIndexes, kNumAdjIndexes, -1);
		mNodeIndexes.push_back(subNodeIndexes);
    }

    ++mNodeIter;
    tile->setGUID(guid);
    for (int i = 0; i < kNumAdjIndexes; ++i)
    {
        subNodeIndexes[i] = -1;

        int nextIndex = topLeftIndex + kColorFillIndexOffsets[i];
        if (mPuzzle->isValidIndex(nextIndex) && !mPuzzle->doesAdjacentPerpIndexWrap(index, nextIndex))
        {
            PuzzleTile* adjacentTile = &mTiles[nextIndex];
            if (adjacentTile->getGUID() != guid && adjacentTile->getGUID() != 0 && !adjacentTile->isModified(PuzzleTile::kTMShielded)) // Root tile will be skipped by GUID test.
            {
                if (adjacentTile->getColorKey() == colorKey)
                {
                    adjacentTile->setDecorator(PuzzleTile::kTFColorFill);
                    adjacentTile->setDecoratorData(fillDepth + 1);
                    adjacentTile->setColorKey(tile->getColorKey());
                    ++numFills;
                }
                else if (adjacentTile->getColorKey() == tile->getColorKey() && adjacentTile->getGUID() != 1) // GUID of 1 means this tile has not been colored during this fill.
                    ++numPreFills;
                else
                    continue;

                adjacentTile->setGUID(guid);
                subNodeIndexes[i] = nextIndex;
            }
        }
    }

    if (numFills == 0 && numPreFills != 0)
    {
        for (int i = 0; i < kNumAdjIndexes; ++i)
        {
            int nextIndex = subNodeIndexes[i];
            if (nextIndex == -1)
                continue;

            PuzzleTile* adjacentTile = &mTiles[nextIndex];
            int nodeFills = fillNode(nextIndex, adjacentTile, colorKey, fillDepth + 1, guid);
            numFills += nodeFills;

            if (nodeFills == 0)
                adjacentTile->setGUID(0); // Mark as an invalid branch.
        }
    }

    --mNodeIter;
    return numFills;
}
