
#include "PoolIndexer.h"
#include <climits>
USING_NS_CC;

PoolIndexer::PoolIndexer(int capacity)
{
	CCAssert(capacity > 0 && capacity < INT_MAX, "Invalid PoolIndexer capacity.");
	mCapacity = capacity;
	mIndices = new int[capacity];
	mIndicesIndex = 0;
}

PoolIndexer::~PoolIndexer(void)
{
	if (mIndices)
	{
		delete[] mIndices;
		mIndices = 0;
	}
}

void PoolIndexer::initIndexes(int startIndex, int increment)
{
	CCAssert(startIndex >= 0 && (startIndex + (increment * mCapacity) >= 0), "Bad PoolIndexer initialization.");

	for (int i = 0; i < mCapacity; ++i)
		mIndices[i] = startIndex + i * increment;
}

void PoolIndexer::insertPoolIndex(int index, int poolIndex)
{
	if (index >= 0 && index < mCapacity)
		mIndices[index] = poolIndex;
}

int PoolIndexer::checkoutNextIndex(void)
{
	if (mIndicesIndex < mCapacity)
        return mIndices[mIndicesIndex++];
    else
        return -1;
}

void PoolIndexer::checkinIndex(int index)
{
	CCAssert(index >= 0 && mIndicesIndex > 0 && mIndicesIndex <= mCapacity, "Bad PoolIndexer checkin.");

	if (index >= 0 && mIndicesIndex > 0 && mIndicesIndex <= mCapacity)
    {
        --mIndicesIndex;
        mIndices[mIndicesIndex] = index;
    }
}
