#ifndef __ISEARCH_PROVIDER_H__
#define __ISEARCH_PROVIDER_H__

#include <Utils/CMTypes.h>

class ISearchProvider
{
public:
	virtual ~ISearchProvider(void) { }
	virtual int getSuccessors(const Coord& tilePos, const Coord& parentPos, Coord* successors, int numAncestors) = 0;
	virtual const Coord& getDxy(void) const = 0;
	virtual int getSearchWeighting(const Coord& tilePos) const = 0;
    virtual int getMaxPathLength() const = 0;
};
#endif // __ISEARCH_PROVIDER_H__
