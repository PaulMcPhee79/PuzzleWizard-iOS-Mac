
#include "Juggler.h"
#include <algorithm>
USING_NS_CC;

typedef std::vector<std::vector<IAnimatable*> > BaseCache;
typedef std::vector<IAnimatable*> SubCache;

Juggler::Juggler(int cacheIncrement)
:
mIsJuggling(false),
mAddsQueued(false),
mQueuedReserveSize(0),
mCacheIncrement(MAX(10, cacheIncrement)),
mTotalCacheSize(0),
mElapsedTime(0)
{
    mTotalCacheCapacity = mCacheIncrement;
    
    for (int i = 0; i < kNumAnimIndexes; ++i)
    {
        mAnimatables[i].reserve(25);
        mAnimatables[i].push_back(SubCache(mCacheIncrement, static_cast<IAnimatable*>(0)));
    }
}

Juggler::~Juggler(void)
{
    for (int i = 0; i < kNumAnimIndexes; ++i)
    {
        for (BaseCache::iterator bit = mAnimatables[i].begin(); bit != mAnimatables[i].end(); ++bit)
        {
            for (SubCache::iterator sit = bit->begin(); sit != bit->end(); ++sit)
                *sit = static_cast<IAnimatable*>(0);
        }
    }
}

void Juggler::reserve(int numCacheIncrements)
{
    if (mIsJuggling)
        mQueuedReserveSize += numCacheIncrements;
    else
    {
        for (int i = 0, n = numCacheIncrements + mQueuedReserveSize; i < n; ++i)
            expandCache();
        mQueuedReserveSize = 0;
    }
}

void Juggler::advanceTime(float dt)
{
	setElapsedTime(getElapsedTime() + dt);

    mIsJuggling = true;
    for (BaseCache::iterator bit = mAnimatables[kAnimIndex].begin(); bit != mAnimatables[kAnimIndex].end(); ++bit)
    {
        for (SubCache::iterator sit = bit->begin(); sit != bit->end(); ++sit)
        {
            if (*sit)
                (*sit)->advanceTime(dt);
        }
    }
    mIsJuggling = false;
    
    if (mQueuedReserveSize != 0)
        reserve(mQueuedReserveSize);
    
    // Can safely add here
    if (mAddsQueued)
    {
        for (BaseCache::iterator bit = mAnimatables[kQueueIndex].begin(); bit != mAnimatables[kQueueIndex].end(); ++bit)
        {
            for (SubCache::iterator sit = bit->begin(); sit != bit->end(); ++sit)
            {
                if (*sit)
                {
                    SubCache::iterator freePosIt = nextAvailablePosition();
                    *freePosIt = *sit;
                    *sit = 0;
                }
            }
        }
        mAddsQueued = false;
    }
}

IAnimatable* Juggler::find(IAnimatable* obj, bool remove)
{
    if (obj)
    {
        for (int i = 0; i < kNumAnimIndexes; ++i)
        {
            for (BaseCache::iterator bit = mAnimatables[i].begin(); bit != mAnimatables[i].end(); ++bit)
            {
                SubCache::iterator sit = std::find(bit->begin(), bit->end(), obj);
                if (sit != bit->end())
                {
                    IAnimatable* found = *sit;
                    if (remove)
                        *sit = static_cast<IAnimatable*>(0);
                    return found;
                }
            }
        }
    }
    
    return NULL;
}

std::vector<IAnimatable*>* Juggler::expandCache(void)
{
    SubCache* retval = NULL;
    if (mIsJuggling)
    {
        mAnimatables[kQueueIndex].push_back(SubCache(mCacheIncrement, static_cast<IAnimatable*>(0)));
        retval = &mAnimatables[kQueueIndex].back();
    }
    else
    {
        mAnimatables[kAnimIndex].push_back(SubCache(mCacheIncrement, static_cast<IAnimatable*>(0)));
        retval = &mAnimatables[kAnimIndex].back();
    }
    
    mTotalCacheCapacity += mCacheIncrement;
    CCLog("Juggler::expandCache to %d", mTotalCacheCapacity);
    return retval;
}

std::vector<IAnimatable*>::iterator Juggler::nextAvailablePosition(void)
{
    BaseCache* baseCache = mIsJuggling ? &mAnimatables[kQueueIndex] : &mAnimatables[kAnimIndex];
    for (BaseCache::iterator bit = baseCache->begin(); bit != baseCache->end(); ++bit)
    {
        for (SubCache::iterator sit = bit->begin(); sit != bit->end(); ++sit)
        {
            if (*sit == static_cast<IAnimatable*>(0))
            {
                if (baseCache == &mAnimatables[kQueueIndex])
                    mAddsQueued = true;
                return sit;
            }
        }
    }
    
    expandCache();
    return nextAvailablePosition();
}

void Juggler::addObject(IAnimatable* obj)
{
	if (obj && !find(obj))
    {
        SubCache::iterator freePosIt = nextAvailablePosition();
        *freePosIt = obj;
        ++mTotalCacheSize;
    }
}

void Juggler::removeObject(IAnimatable* obj)
{
    if (obj && find(obj, true))
        --mTotalCacheSize;
}

void Juggler::removeAllObjects(void)
{
    for (int i = 0; i < kNumAnimIndexes; ++i)
    {
        for (BaseCache::iterator bit = mAnimatables[i].begin(); bit != mAnimatables[i].end(); ++bit)
        {
            for (SubCache::iterator sit = bit->begin(); sit != bit->end(); ++sit)
                *sit = static_cast<IAnimatable*>(0);
        }
    }
    
    mTotalCacheSize = 0;
}

void Juggler::removeTweensWithTarget(void* target)
{
	if (target == NULL) return;
    
    for (int i = 0; i < kNumAnimIndexes; ++i)
    {
        for (BaseCache::iterator bit = mAnimatables[i].begin(); bit != mAnimatables[i].end(); ++bit)
        {
            for (SubCache::iterator sit = bit->begin(); sit != bit->end(); ++sit)
            {
                if (*sit && (*sit)->getTarget() == target)
                {
                    *sit = static_cast<IAnimatable*>(0);
                    --mTotalCacheSize;
                }
            }
            
        }
    }
}

void Juggler::printContents(void)
{
#if DEBUG
    int index = 0;
    for (BaseCache::iterator bit = mAnimatables[kAnimIndex].begin(); bit != mAnimatables[kAnimIndex].end(); ++bit)
    {
        for (SubCache::iterator sit = bit->begin(); sit != bit->end(); ++sit, ++index)
            CCLog("Juggler[%d]:%p -> %s", index, *sit, *sit ? (*sit)->getAnimatableName().c_str() : "NULL");
    }

    CCLog("Juggler size:%d/%d", mTotalCacheSize, mTotalCacheCapacity);
    CCLog("<<<-------------------->>>");
#endif
}
