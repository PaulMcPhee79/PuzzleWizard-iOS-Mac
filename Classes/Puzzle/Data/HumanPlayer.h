#ifndef __HUMAN_PLAYER_H__
#define __HUMAN_PLAYER_H__

#include "cocos2d.h"
#include "Player.h"
#include "MirroredPlayer.h"
USING_NS_CC;

class HumanPlayer : public Player
{
public:
	HumanPlayer(uint colorKey, Coord position = CMCoordZero, uint orientation = kSouthernOrientation);
	virtual ~HumanPlayer(void);

	static HumanPlayer* create(uint colorKey, Coord position = CMCoordZero, uint orientation = kSouthernOrientation, bool autorelease = true);

	virtual PlayerType getType(void) { return Player::HUMAN_PLAYER; }
	virtual void broadcastProperties(void);
	virtual void beginMoveTo(const Coord& pos);
    virtual void treadmill(const Coord& dir);
    virtual void idle(void);
    virtual void reset(void);
    virtual void didFinishMoving(void);
	virtual HumanPlayer* clone(void) const;
	virtual HumanPlayer* devClone(void) const;
	
	MirroredPlayer* getMirroredSelf() const { return mMirroredSelf; }
	void setMirroredSelf(MirroredPlayer* value)
	{
		if (mMirroredSelf == value)
			return;

		CC_SAFE_RETAIN(value);
		CC_SAFE_RELEASE(mMirroredSelf);
		mMirroredSelf = value;
	}

private:
	HumanPlayer(const HumanPlayer& other);
	HumanPlayer& operator=(const HumanPlayer& rhs);

	MirroredPlayer* mMirroredSelf;
};
#endif // __HUMAN_PLAYER_H__
