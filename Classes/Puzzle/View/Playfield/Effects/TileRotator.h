#ifndef __TILE_ROTATOR_H__
#define __TILE_ROTATOR_H__

#include "cocos2d.h"
#include <Prop/Prop.h>
#include <Events/EventDispatcher.h>
#include <Interfaces/IEventListener.h>
#include <Interfaces/IReusable.h>
class TilePiece;
class TileShadow;
class IEventListener;
class ITileDecorator;
class FloatTweener;
class PlayerPiece;
class ReusableCache;
USING_NS_CC;

class TileRotator : public Prop, public IEventListener, public IReusable
{
public:
	static int EV_TYPE_ANIMATION_COMPLETED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
    
	TileRotator(int category); 
	virtual ~TileRotator(void);

	static TileRotator* createWith(int category, const CCPoint& rotatorOrigin, const CCRect& boardBounds, const std::vector<TilePiece*>& tilePieces, bool autorelease = true);
	bool initWith(const CCPoint& rotatorOrigin, const CCRect& boardBounds, const std::vector<TilePiece*>& tilePieces);
    
    void setEdgeStatus(int value); // OR'ed PuzzleEffects::EdgeStatus

	virtual void onEvent(int evType, void* evData);

	virtual bool isComplete(void) const { return mHasAnimated && mState == STATIONARY; }
	virtual void advanceTime(float dt);

	void addPivot(TilePiece* tilePiece, PlayerPiece* playerPiece);
	void addPlayerPiece(int index, PlayerPiece* playerPiece);
	void animate(float duration);
	void stopAnimating(void);

	static TileRotator* getTileRotator(const CCPoint& rotatorOrigin, const CCRect& boardBounds, const std::vector<TilePiece*>& tilePieces);
	static void setupReusables(void);
    virtual void returnToPool(void);

	ITileDecorator* getDecorator(void) const { return mDecorator; }
	void setDecorator(ITileDecorator* value);

	IEventListener* getListener(void) const { return mListener; }
	void setListener(IEventListener* value) { mListener = value; }
    
    static void setShadowOccOverlap(float value);

protected:

private:
	enum RotatorState { STATIONARY = 0, RAISING, ROTATING, DROPPING };
    
    static float s_shadowOccOverlap;

    CCRect getShadowOcclusionFromBounds(const CCRect& bounds);
    virtual int getReuseKey(void) { return 0; }
	virtual void reuse(void);
    virtual void hibernate(void);
	static IReusable* checkoutReusable(uint reuseKey);
	static void checkinReusable(IReusable* reusable);

	static ReusableCache* sCache;
    static bool sCaching;

	RotatorState getState(void) const { return mState; }
	void setState(RotatorState value);
	void raiseTiles(void);
	void rotateTiles(void);
	void dropTiles(void);
	int getBaseIndexFromTag(int tag) const;
	void reconfigure(const CCPoint& rotatorOrigin, const CCRect& boardBounds, const std::vector<TilePiece*>& tilePieces);

	bool mHasAnimated;
    RotatorState mState;
    int mEdgeStatus;
    float mDuration;

	int mRaiseCompletedTag;
	int mRotateCompletedTag;
	int mDropCompletedTag;

    CCPoint mTileDimensions;
    CCRect mShadowOcclusionRegion;

	TilePiece* mPivotPiece;
	PlayerPiece* mPivotPlayer;

    std::vector<TilePiece*> mTiles;
    std::vector<TileShadow*> mTileShadows;
    CCNode* mLayers[2];

    std::vector<FloatTweener*> mRaiseTweens;
    std::vector<FloatTweener*> mRotateTweens;
    std::vector<FloatTweener*> mDropTweens;

	ITileDecorator* mDecorator;
	IEventListener* mListener;
};
#endif // __TILE_ROTATOR_H__
