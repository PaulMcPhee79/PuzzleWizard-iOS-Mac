#ifndef __POOL_INDEXER_H__
#define __POOL_INDEXER_H__

#include "cocos2d.h"
USING_NS_CC;

class PoolIndexer : public CCObject
{
public:
	PoolIndexer(int capacity);
	virtual ~PoolIndexer(void);

	void initIndexes(int startIndex, int increment);
	void insertPoolIndex(int index, int poolIndex);
	int checkoutNextIndex(void);
	void checkinIndex(int index);
	
	int getCapacity(void) const { return mCapacity; }

private:
	int mCapacity;
	int mIndicesIndex;
	int* mIndices;
};
#endif // __POOL_INDEXER_H__
