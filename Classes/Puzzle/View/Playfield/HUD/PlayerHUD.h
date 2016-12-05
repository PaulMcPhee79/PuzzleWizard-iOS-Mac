#ifndef __PLAYER_HUD_H__
#define __PLAYER_HUD_H__

#include "cocos2d.h"
#include <Prop/Prop.h>
#include <Interfaces/IPlayerView.h>
#include <Interfaces/IEventListener.h>
#include <Interfaces/ILocalizable.h>
class ByteTweener;
class Player;
class HUDCell;
USING_NS_CC;

class PlayerHUD : public Prop, public IPlayerView, public IEventListener, public ILocalizable
{
public:
	static const uint kHUDRed = 0xdb0000;
	static const uint kHUDAmber = 0xfffc00;
	static const uint kHUDGreen = 0x48db00;

	PlayerHUD(int category, const CCRect& bounds);
	virtual ~PlayerHUD(void);

	static PlayerHUD* create(int category, const CCRect& bounds, bool autorelease = true);
	virtual bool init(void);
    
    virtual void localeDidChange(const char* fontKey, const char* FXFontKey);

	virtual void playerValueDidChange(uint code, int value);
	virtual void onEvent(int evType, void* evData);
	virtual void advanceTime(float dt);

	Player* getPlayer(void) const { return mPlayer; }
	void setPlayer(Player* value);

private:
	enum HUDState { HS_OUT, HS_TRANSITION_IN, HS_IN, HS_TRANSITION_OUT };

	HUDState getState(void) const { return mState; }
	void setState(HUDState value);
	std::string getHudString(int value) const;
	uint getTextColorForValue(int value, int range);


	CCRect mOriginBounds;
	CCSprite* mBgQuad;
	ByteTweener* mTransitionTweener;

    int mPrevPlayerValue;
	HUDState mState;
	Player* mPlayer;
	HUDCell* mHUDCell;
};
#endif // __PLAYER_HUD_H__
