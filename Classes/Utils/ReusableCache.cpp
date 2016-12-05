
#include "ReusableCache.h"
#include "PoolIndexer.h"
USING_NS_CC;

typedef std::map<int, std::vector<IReusable*>*> Caches;
typedef std::vector<IReusable*> SubCache;
typedef std::map<int, PoolIndexer*> Indexers;

ReusableCache::ReusableCache(void)
{

}

ReusableCache::~ReusableCache(void)
{
	purge();
}

void ReusableCache::addKey(unsigned int qty, int key)
{
	if (mCache[key])
        return;

	mCache[key] = new SubCache();
	mCache[key]->reserve(qty);
	PoolIndexer* poolIndexer = new PoolIndexer((int)qty);
	poolIndexer->initIndexes(0, 1);
	mIndexers[key] = poolIndexer;
}

void ReusableCache::addReusable(IReusable* reusable)
{
	if (reusable == NULL || mCache[reusable->getReuseKey()] == NULL)
        return;

	SubCache* subCache = mCache[reusable->getReuseKey()];
	subCache->push_back(reusable);
}

void ReusableCache::verifyCacheIntegrity()
{
	for(Caches::iterator it = mCache.begin(); it != mCache.end(); ++it)
	{
		SubCache* subCache = it->second;
		PoolIndexer* poolIndexer = mIndexers[it->first];
		CCAssert(subCache && poolIndexer && (int)subCache->size() == poolIndexer->getCapacity(), "ReusableCache failed data integrity test.");
		if (subCache == NULL || poolIndexer == NULL)
			break;
	}
}

IReusable* ReusableCache::checkout(int key)
{
	IReusable* checkout = NULL;
	SubCache* subCache = mCache[key];
	PoolIndexer* poolIndexer = mIndexers[key];
	if (subCache && poolIndexer)
	{
		int index = poolIndexer->checkoutNextIndex();
		if (index != -1)
		{
			checkout = (*subCache)[index];
			checkout->setPoolIndex(index);
		}
	}

    return checkout;
}

void ReusableCache::checkin(IReusable* reusable)
{
	CCAssert(reusable && !reusable->getInUse() && reusable->getPoolIndex() != -1, "Attempt to Checkin IReusable with invalid state.");
	if (reusable == NULL || reusable->getInUse() || reusable->getPoolIndex() == -1)
		return;

	PoolIndexer* poolIndexer = mIndexers[reusable->getReuseKey()];
	if (poolIndexer)
		poolIndexer->checkinIndex(reusable->getPoolIndex());
	reusable->setPoolIndex(-1);
}

// Note: IReusables leak from here, although we currently never purge and always want
// them around for the life of the program, but if this changes, then add an IEventListener
// client to IReusable that it can call back to and then the callee can autorelease itself without
// having to cast IResuable to CCObject (which is an unpredictable cast and ill-advised).
void ReusableCache::purge(void)
{
	for(Caches::iterator it = mCache.begin(); it != mCache.end(); ++it)
		delete[] it->second;
	mCache.clear();
	
	for(Indexers::iterator it = mIndexers.begin(); it != mIndexers.end(); ++it)
		CC_SAFE_RELEASE(it->second);
	mIndexers.clear();
}
