#ifndef __TILE_SWAPPER_H__
#define __TILE_SWAPPER_H__

#include "cocos2d.h"
#include <Prop/Prop.h>
#include <Events/EventDispatcher.h>
#include <Interfaces/IEventListener.h>
#include <Interfaces/IReusable.h>
#include <Utils/CMTypes.h>
class TilePiece;
class ReusableCache;
class Dissolver;
USING_NS_CC;

class TileSwapper : public Prop, public IEventListener, public IReusable
{
public:
	static int EV_TYPE_ANIMATION_COMPLETED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }

	TileSwapper(int category);
	virtual ~TileSwapper(void);

	static TileSwapper* createWith(int category, const vec4& swapOrigins, const std::vector<TilePiece*>& tilePieces, bool autorelease = true);
	bool initWith(const vec4& swapOrigins, const std::vector<TilePiece*>& tilePieces);

	virtual void onEvent(int evType, void* evData);

	virtual bool isComplete(void) const { return mHasStoppedAnimating; }
	virtual void advanceTime(float dt);
	void stopAnimating(void);

	static TileSwapper* getTileSwapper(const vec4& swapOrigins, const std::vector<TilePiece*>& tilePieces);
	static void setupReusables(void);
    virtual void returnToPool(void);

	IEventListener* getListener(void) const { return mListener; }
	void setListener(IEventListener* value) { mListener = value; }

private:
    virtual int getReuseKey(void) { return 0; }
	virtual void reuse(void);
    virtual void hibernate(void);
	static IReusable* checkoutReusable(uint reuseKey);
	static void checkinReusable(IReusable* reusable);
	void reconfigure(const vec4& swapOrigins, const std::vector<TilePiece*>& tilePieces);

	static ReusableCache* sCache;
    static bool sCaching;

	bool mHasStoppedAnimating;
	float mThreshold;
	std::vector<TilePiece*> mTilePieces;
	Dissolver* mDissolver;

    IEventListener* mListener;
};
#endif // __TILE_SWAPPER_H__
