#ifndef __REUSABLE_CACHE_H__
#define __REUSABLE_CACHE_H__

#include "cocos2d.h"
#include <Interfaces/IReusable.h>
class PoolIndexer;
USING_NS_CC;

class ReusableCache : CCObject
{
public:
	ReusableCache(void);
	virtual ~ReusableCache(void);

	void addKey(unsigned int qty, int key);
	void addReusable(IReusable* reusable);
	void verifyCacheIntegrity();
	IReusable* checkout(int key);
	void checkin(IReusable* reusable);
	void purge(void);

private:
	std::map<int, std::vector<IReusable*>*> mCache;
	std::map<int, PoolIndexer*> mIndexers;
};
#endif // __REUSABLE_CACHE_H__
