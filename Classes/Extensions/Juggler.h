#ifndef __JUGGLER_H__
#define __JUGGLER_H__

#include "cocos2d.h"
#include <Interfaces/IAnimatable.h>
USING_NS_CC;

class Juggler : public CCObject, public IAnimatable
{
public:
	Juggler(int cacheIncrement = 10);
	virtual ~Juggler(void);

    void reserve(int numCacheIncrements);
	float getElapsedTime(void) const { return mElapsedTime; }
	virtual void advanceTime(float dt);
	void addObject(IAnimatable* obj);
	void removeObject(IAnimatable* obj);
	void removeAllObjects(void);
	void removeTweensWithTarget(void* target); // Note: Make sure you cast to void* from the EXACT same type as your IAnimatable::getTarget() does.
    void printContents(void);

private:
	void setElapsedTime(float value) { mElapsedTime = value; }
    IAnimatable* find(IAnimatable* obj, bool remove = false);
    std::vector<IAnimatable*>* expandCache(void);
    std::vector<IAnimatable*>::iterator nextAvailablePosition(void);
    
    static const int kAnimIndex = 0;
    static const int kQueueIndex = 1;
    static const int kNumAnimIndexes = 2;
    
    bool mIsJuggling;
    bool mAddsQueued;
    int mQueuedReserveSize;
    int mCacheIncrement;
    int mTotalCacheSize;
    int mTotalCacheCapacity;
	float mElapsedTime;
    std::vector<std::vector<IAnimatable*> > mAnimatables[kNumAnimIndexes];
    
};
#endif // __JUGGLER_H__
