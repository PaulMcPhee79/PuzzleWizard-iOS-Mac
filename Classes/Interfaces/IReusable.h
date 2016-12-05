#ifndef __IREUSABLE_H__
#define __IREUSABLE_H__

class IReusable
{
public:
	IReusable(void) : mInUse(true), mReuseKey(0), mPoolIndex(-1) { }
	virtual ~IReusable(void) { }

	virtual bool getInUse(void) { return mInUse; };
	virtual int getReuseKey(void) { return mReuseKey; }
	virtual int getPoolIndex(void) { return mPoolIndex; }
	virtual void setPoolIndex(int value) { mPoolIndex = value; }

    virtual void reuse(void) = 0;
    virtual void hibernate(void) = 0;
    virtual void returnToPool(void) = 0;

protected:
	bool mInUse;
	int mReuseKey;
	int mPoolIndex;
};
#endif // __IREUSABLE_H__
