#ifndef __TILE_CONVEYOR_BELT_H__
#define __TILE_CONVEYOR_BELT_H__

#include "cocos2d.h"
#include <Prop/Prop.h>
#include <Events/EventDispatcher.h>
#include <Interfaces/IEventListener.h>
#include <Interfaces/IReusable.h>
#include <Utils/CMTypes.h>
class TilePiece;
class PlayerPiece;
class TileShadow;
class ITileDecorator;
class FloatTweener;
class ReusableCache;
class CroppedProp;
USING_NS_CC;

class TileConveyorBelt : public Prop, public IEventListener, public IReusable
{
public:
	static int EV_TYPE_ANIMATION_COMPLETED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }

	TileConveyorBelt(int category);
	virtual ~TileConveyorBelt(void);

	static TileConveyorBelt* createWith(int category, const CCRect& boardBounds, Coord animDir, int wrapIndex, std::vector<TilePiece*>& beltTiles, bool autorelease = true);
	bool initWith(const CCRect& boardBounds, Coord animDir, int wrapIndex, std::vector<TilePiece*>& beltTiles);

    void setEdgeStatus(int value) { mEdgeStatus = value; } // OR'ed PuzzleEffects::EdgeStatus
    
	virtual void onEvent(int evType, void* evData);

	virtual bool isComplete(void) const { return mHasStoppedAnimating; }
	virtual void advanceTime(float dt);
	virtual void visit(void);

	void addPlayerPiece(int index, PlayerPiece* playerPiece);
	void animate(float duration);
	void stopAnimating(void);

	static TileConveyorBelt* getTileConveyorBelt(const CCRect& boardBounds, Coord animDir, int wrapIndex, std::vector<TilePiece*>& beltTiles);
	static void setupReusables(void);
    virtual void returnToPool(void);

	ITileDecorator* getDecorator(void) const { return mDecorator; }
	void setDecorator(ITileDecorator* value);

	IEventListener* getListener(void) const { return mListener; }
	void setListener(IEventListener* value) { mListener = value; }

private:
	enum CBeltState { STATIONARY = 0, RAISING, SLIDING, DROPPING };
	static const int kOccLayerIndex = 0;
	static const int kPlayerLayerIndex = 1;
	static const int kWrapLayerIndex = 2;
	static const int kNumLayers = kWrapLayerIndex + 1;

    CCRect getOcclusionOverlap(int occIndex);
    virtual int getReuseKey(void) { return getOrientation(); }
	virtual void reuse(void);
    virtual void hibernate(void);
	static IReusable* checkoutReusable(uint reuseKey);
	static void checkinReusable(IReusable* reusable);
	void reconfigure(const CCRect& boardBounds, Coord animDir, int wrapIndex, std::vector<TilePiece*>& beltTiles);

	static ReusableCache* sCache;
    static bool sCaching;

	uint getOrientation(void) const;
	CBeltState getState(void) const { return mState; }
	void setState(CBeltState value);
    void syncWrapShadowPosition(void);

	float getScaleWrapProxy(void);
	void setScaleWrapProxy(float value);
	float getMoveWrapProxy(void);
	void setMoveWrapProxy(float value);
	float getRaiseWrapProxy(void);
	void setRaiseWrapProxy(float value);
	float getMoveBeltProxy(void);
	void setMoveBeltProxy(float value);

	void raiseWrapTile(void);
	void slideWrapTile(void);
	void slideBeltTiles(void);
	void dropWrapTile(void);
	

	bool mHasAnimated;
    bool mHasStoppedAnimating;
    CBeltState mState;
    int mEdgeStatus;
    int mWrapIndex;
    Coord mAnimDir;
    float mDuration;
    float mRaiseOffsetY;
    float mShadowBaseScale;
    std::vector<TilePiece*> mBeltTilePieces;
    std::vector<TilePiece*> mOcclusionPieces;

	TilePiece* mWrapTile;
    TileShadow* mWrapShadow;
    CroppedProp* mWrapShadowCrop;
	CCNode* mLayers[kNumLayers];

    std::map<int, PlayerPiece*> mPlayerPieces;

    float mBeltMovement;
    FloatTweener* mMoveBeltTweener;
    FloatTweener* mMoveWrapTweener;
    FloatTweener* mScaleWrapTweener;
	FloatTweener* mRaiseWrapTweener;

	ITileDecorator* mDecorator;
    IEventListener* mListener;
};
#endif // __TILE_CONVEYOR_BELT_H__
