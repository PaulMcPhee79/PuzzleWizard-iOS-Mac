
#include "HumanPlayer.h"
USING_NS_CC;

HumanPlayer::HumanPlayer(uint colorKey, Coord position, uint orientation)
	: Player(colorKey, position, orientation), mMirroredSelf(NULL)
{

}

HumanPlayer::~HumanPlayer(void)
{
	CC_SAFE_RELEASE_NULL(mMirroredSelf);
}

HumanPlayer* HumanPlayer::create(uint colorKey, Coord position, uint orientation, bool autorelease)
{
	HumanPlayer *player = new HumanPlayer(colorKey, position, orientation);
    if (player)
    {
		if (autorelease)
			player->autorelease();
        return player;
    }
    CC_SAFE_DELETE(player);
    return NULL;
}

void HumanPlayer::broadcastProperties(void)
{
	Player::broadcastProperties();

	if (isColorMagicActive())
		notifyPropertyChange(kValueColorMagic, getNumColorMagicMoves());
}

void HumanPlayer::beginMoveTo(const Coord& pos)
{
	Coord prevPos = getPosition();
    Player::beginMoveTo(pos);

    if (mMirroredSelf)
		mMirroredSelf->setQueuedMove(cmc(prevPos.x - pos.x, prevPos.y - pos.y));
}

void HumanPlayer::treadmill(const Coord& dir)
{
    Player::treadmill(dir);
    
    if (mMirroredSelf)
		mMirroredSelf->treadmill(cmc(-dir.x, -dir.y));
}

void HumanPlayer::idle(void)
{
    Player::idle();
    if (mMirroredSelf)
        mMirroredSelf->mirrorIdle();
}

void HumanPlayer::reset(void)
{
    Player::reset();
    setMirroredSelf(NULL);
}

void HumanPlayer::didFinishMoving(void)
{
    Player::didFinishMoving();
    if (mMirroredSelf)
    {
        // Puzzle state changes on move end, so sync players that they may react to a common state.
        if (mMirroredSelf->isMoving())
            mMirroredSelf->didFinishMoving();
    }
}

HumanPlayer* HumanPlayer::clone(void) const
{
	HumanPlayer* player = new HumanPlayer(getColorKey(), getPosition(), getOrientation());
	player->autorelease();
	return player;
}

HumanPlayer* HumanPlayer::devClone(void) const
{
	HumanPlayer* player = new HumanPlayer(getDevColorKey(), getDevPosition(), getDevOrientation());
	player->autorelease();
	return player;
}

