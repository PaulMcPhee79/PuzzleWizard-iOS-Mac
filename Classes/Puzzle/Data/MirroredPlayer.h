#ifndef __MIRRORED_PLAYER_H__
#define __MIRRORED_PLAYER_H__

#include "cocos2d.h"
#include "Player.h"
USING_NS_CC;

class MirroredPlayer : public Player
{
public:
	MirroredPlayer(uint colorKey, Coord position = CMCoordZero, uint orientation = kSouthernOrientation);

	static MirroredPlayer* create(uint colorKey, Coord position = CMCoordZero, uint orientation = kSouthernOrientation, bool autorelease = true);

	virtual PlayerType getType(void) { return Player::MIRRORED_MATE; }
	virtual const char* getCostumeName() const { return "mirrored-player"; }
	virtual void broadcastProperties(void);
	virtual void moveTo(const Coord& pos);
	virtual void reset(void);
	virtual MirroredPlayer* clone(void) const;
	virtual MirroredPlayer* devClone(void) const;
	

	bool getHasExpired() const { return getNumMovesRemaining() <= 0; }
	int getNumMovesRemaining() const { return mNumMovesRemaining; }
	void setNumMovesRemaining(int value)
	{
		mNumMovesRemaining = value;
		notifyPropertyChange(kValueMirroredMate, value);
	}
    
    virtual void idle(void);
    void mirrorIdle(void);
protected:

private:
	MirroredPlayer(const MirroredPlayer& other);
	MirroredPlayer& operator=(const MirroredPlayer& rhs);

	int mNumMovesRemaining;
};
#endif // __MIRRORED_PLAYER_H__
