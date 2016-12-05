#ifndef __DISSOLVER_H__
#define __DISSOLVER_H__

#include "cocos2d.h"
#include <Prop/Prop.h>
#include <Events/EventDispatcher.h>
class IEventListener;
USING_NS_CC;

class Dissolver : public Prop
{
public:
	static int EV_TYPE_DISSOLVE_CYCLE_COMPLETED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }

	// dir == 0 to cycle
	Dissolver(int category, int dir = 1, float minThreshold = 0.0f, float maxThreshold = 1.0f);
	virtual ~Dissolver(void);

	static Dissolver* createWith(int category, CCTexture2D* batchTex, int dir = 1, float minThreshold = 0.0f, float maxThreshold = 1.0f, bool autorelease = true);
	bool initWith(CCTexture2D* batchTex, uint capacity = 20);

	void addDissolvee(CCSprite* dissolvee);
	void removeDissolvee(CCSprite* dissolvee);
	void clear();
	void reset();

	virtual bool isComplete(void) const { return (mDir == 1 && mThreshold == mMaxThreshold) || (mDir == -1 && mThreshold == mMinThreshold); }
	virtual void advanceTime(float dt);
	virtual void visit(void);

	float getThreshold(void) const { return mThreshold; }
	void setThreshold(float value) { mThreshold = value; }

	IEventListener* getListener(void) const { return mListener; }
	void setListener(IEventListener* value) { mListener = value; }

private:
	int mRequestedDir;
	int mDir;
	float mMinThreshold;
	float mMaxThreshold;
	float mThreshold;
	CCSpriteBatchNode* mBatch;
	IEventListener* mListener;
};
#endif // __DISSOLVER_H__
