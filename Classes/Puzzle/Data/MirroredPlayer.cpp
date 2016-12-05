
#include "MirroredPlayer.h"
#include <Utils/DeviceDetails.h>
USING_NS_CC;

MirroredPlayer::MirroredPlayer(uint colorKey, Coord position, uint orientation)
	: Player(colorKey, position, orientation)
{
    mNumMovesRemaining = MODE_8x6 ? 8 : 10;
}

MirroredPlayer* MirroredPlayer::create(uint colorKey, Coord position, uint orientation, bool autorelease)
{
	MirroredPlayer *player = new MirroredPlayer(colorKey, position, orientation);
    if (player)
    {
		if (autorelease)
			player->autorelease();
        return player;
    }
    CC_SAFE_DELETE(player);
    return NULL;
}

void MirroredPlayer::broadcastProperties(void)
{
	Player::broadcastProperties();
	notifyPropertyChange(kValueMirroredMate, getNumMovesRemaining());
}

void MirroredPlayer::moveTo(const Coord& pos)
{
	if (getNumMovesRemaining() <= 0)
        return;

    Player::moveTo(pos);

    setNumMovesRemaining(getNumMovesRemaining()-1);
}

void MirroredPlayer::idle(void)
{
    // Do nothing - don't call base class. HumanPlayer will idle us via MirroredPlayer::mirrorIdle.
}

void MirroredPlayer::mirrorIdle(void)
{
    Player::idle();
}

void MirroredPlayer::reset(void)
{
	Player::reset();
    setNumMovesRemaining(0);
}

MirroredPlayer* MirroredPlayer::clone(void) const
{
	MirroredPlayer* player = new MirroredPlayer(getColorKey(), getPosition(), getOrientation());
	player->autorelease();
	return player;
}

MirroredPlayer* MirroredPlayer::devClone(void) const
{
	MirroredPlayer* player = new MirroredPlayer(getDevColorKey(), getDevPosition(), getDevOrientation());
	player->autorelease();
	return player;
}
