#ifndef __CM_LOCK_H__
#define __CM_LOCK_H__

#include <pthread.h>

class CMLock
{
public:
	explicit CMLock(pthread_mutex_t* mutex, bool blocking = false);
	~CMLock(void);
    
    bool isLocked(void) const { return mLocked; }
    
private:
    CMLock();
    CMLock(const CMLock& other);
	CMLock& operator=(const CMLock& rhs);
    
    bool mLocked;
    pthread_mutex_t* mMutex;
};
#endif // __CM_LOCK_H__
