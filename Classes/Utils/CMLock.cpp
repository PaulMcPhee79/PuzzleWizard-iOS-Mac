
#include "CMLock.h"
#include <assert.h>

CMLock::CMLock(pthread_mutex_t* mutex, bool blocking)
: mLocked(false)
, mMutex(mutex)
{
    assert(mMutex);
    if (!blocking)
        mLocked = pthread_mutex_trylock(mMutex) == 0;
    else
        mLocked = pthread_mutex_lock(mMutex) == 0;
}

CMLock::~CMLock(void)
{
    if (mLocked)
    {
        int errCode = pthread_mutex_unlock(mMutex);
        assert(errCode == 0);
    }
}
