#ifndef __SHIELD_H__
#define __SHIELD_H__

#include "cocos2d.h"
#include <Prop/Prop.h>
#include <Events/EventDispatcher.h>
#include <Interfaces/IEventListener.h>
#include <Interfaces/IReusable.h>
#include <Utils/CMTypes.h>
class ReusableCache;
class FloatTweener;
class ByteTweener;
USING_NS_CC;

class Shield : public Prop, public IEventListener, public IReusable
{
public:
	enum ShieldType { NORMAL, TOP, BOTTOM };

	static int EV_TYPE_DEPLOYING() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
	static int EV_TYPE_DEPLOYED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
	static int EV_TYPE_WITHDRAWING() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
	static int EV_TYPE_WITHDRAWN() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }

	static const float kShieldDomeRadius;
	static const float kShieldDomeDiameter;
	static const GLubyte kDeployedAlpha;
	static const int kMaxFoci = 2;

	Shield(int category, int id, int tileIndex);
	virtual ~Shield(void);

	static Shield* createWith(int category, int id, int tileIndex, const CCPoint& shieldOrigin, bool autorelease = true);
	bool initWith(const CCPoint& shieldOrigin);

	virtual void onEvent(int evType, void* evData);
	virtual void advanceTime(float dt);
	virtual void visit(void);

	void deploy(bool playSound = true);
	void withdraw(bool playSound = true);
	int getID(void) const { return mID; }
	void setID(int value) { mID = value; }
    int getTileIndex(void) const { return mTileIndex; }
	ShieldType getType(void) const { return mType; }
	void setType(ShieldType value);
	bool isDeployed(void) const { return mIsDeployed; }
    
    void setStencil(int index, int texIndex, float texRotation);
    void resetStencils(void);

	static Shield* getShield(int id, int tileIndex, const CCPoint& shieldOrigin);
	static void setupReusables(void);
    virtual void returnToPool(void);
	

	IEventListener* getListener(void) const { return mListener; }
	void setListener(IEventListener* value) { mListener = value; }

private:
	static IReusable* checkoutReusable(uint reuseKey);
	static void checkinReusable(IReusable* reusable);
    virtual int getReuseKey(void) { return 0; }
	virtual void reuse(void);
    virtual void hibernate(void);
	void reconfigure(int id, int tileIndex, const CCPoint& shieldOrigin);

	static ReusableCache* sCache;
    static bool sCaching;

	void notifyListener(uint evCode);
	void cancelTweens(void);
	void setDeployed(bool value) { mIsDeployed = value; }
	void onTweenerChanged(int tag, GLubyte bVal, float fVal);
	void onTweenerCompleted(int tag, GLubyte bVal, float fVal);

	bool mIsDeployed;
	int mID;
    int mTileIndex;
	ShieldType mType;
	float mElapsedTime;
    
    std::string mStencilTextures[2];
    float mStencilRotations[2];
    
	CCSprite* mShieldDome;

	ByteTweener* mAlphaTweeners[2];
	FloatTweener* mScaleTweeners[2];
	
	IEventListener* mListener;
};
#endif // __SHIELD_H__
