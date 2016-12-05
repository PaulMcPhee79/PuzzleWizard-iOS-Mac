#ifndef __PLAYER_PIECE_H__
#define __PLAYER_PIECE_H__

#include "cocos2d.h"
#include <Prop/Prop.h>
#include <Puzzle/Data/Player.h>
#include <Interfaces/IPlayerView.h>
#include <Events/EventDispatcher.h>
#include <Interfaces/IEventListener.h>
#include <Interfaces/IReusable.h>
#include <Utils/CMTypes.h>
class ReusableCache;
class ByteTweener;
USING_NS_CC;

class PlayerPiece : public Prop, public EventDispatcher, public IPlayerView, public IEventListener, public IReusable
{
public:
	enum PPType { STATIC = 0, HUMAN, MIRROR };
	enum PPState { STATIONARY = 0, TREADMILL, MOVING };
	static int EV_TYPE_DID_TRANSITION_OUT() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }

	PlayerPiece(Player* player);
	virtual ~PlayerPiece(void);

	virtual bool init(void);

	virtual void transitionIn(float duration, float delay = 0.0f);
	virtual void transitionOut(float duration);
	virtual void onEvent(int evType, void* evData);
	virtual void enableMenuMode(bool enable) { }
	virtual void setPositionAestheticOnly(const CCPoint& value) { }
    virtual void refreshAesthetics(void) { }
	virtual void syncWithPlayerPiece(PlayerPiece* playerPiece) { }
	virtual void setData(Player* player);
	virtual void syncWithData(void) = 0;
	virtual void moveTo(const Coord& pos);
	virtual void playerValueDidChange(uint code, int value) { syncWithData(); }
	virtual void willBeginMoving(void);
    virtual void didIdle(void);
    virtual void didTreadmill(void);
	void reset();
    
    bool isStationary(void) { return mState != MOVING; }
	PPState getState(void) const { return mState; }
	Player* getPlayer(void) const { return mPlayer; }

    static PlayerPiece* getPlayerPiece(Player* player, bool shouldPlaySounds = true);
	static PlayerPiece* getPlayerPiece(Player* player, PPType type, bool shouldPlaySounds = true);
	static void setupReusables(void);
    virtual void returnToPool(void);

	static float rotationForPlayerOrientation(uint orientation);

protected:
	void setPlayer(Player* value);
	virtual void setState(PPState value) { mState = value; }
	static IReusable* checkoutReusable(uint reuseKey);
	static void checkinReusable(IReusable* reusable);
    
    virtual void reuse(void);
    virtual void hibernate(void);

    PPState mState;
    CCNode* mPlayerCanvas;
    
private:
    virtual int getReuseKey(void) { return 0; }
    
	static ReusableCache* sCache;
    static bool sCaching;

	Player* mPlayer;
	ByteTweener* mTransitionInTweener;
    ByteTweener* mTransitionOutTweener;
};
#endif // __PLAYER_PIECE_H__
