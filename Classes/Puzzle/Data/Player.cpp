
#include "Player.h"
#include <SceneControllers/GameController.h>
#include <Utils/DeviceDetails.h>
#include <Utils/PWDebug.h>
#include <climits>
USING_NS_CC;

typedef std::set<IPlayerView*> PlayerViews;

static const int kMaxColorMagicMoves_8x6 = 4;
static const int kMaxColorMagicMoves_10x8 = 5;

int Player::kMaxColorMagicMoves = -1;

int Player::maxColorMagicMoves(void)
{
    if (kMaxColorMagicMoves == -1)
    {
        kMaxColorMagicMoves = MODE_8x6 ? kMaxColorMagicMoves_8x6 : kMaxColorMagicMoves_10x8;
    }
    
    return kMaxColorMagicMoves;
}

Player::Player(uint colorKey, const Coord& position, uint orientation)
	:
mViewOffset(CMCoordZero),
mQueuedMove(CMCoordZero),
mIsMovingTo(CMCoordZero),
mIsMoving(false),
mDidMove(false),
mViewsLocked(false),
mNumColorMagicMoves(0),
mFunction(0),
mFunctionData(0),
mViews(NULL)
{
    PWDebug::playerCount++;
	mDevColorKey = mColorKey = colorKey;
    mDevOrientation = mOrientation = orientation;
    mDevPosition = mPosition = position;
	mPrevOrientation = mPrevColorKey = mFutureColorKey = 0;
	mID = GameController::GC()->getUnqiueKey();
}

Player::~Player(void)
{
	delete mViews, mViews = NULL;
}

uint Player::getMirroredOrientation(uint orientation)
{
	switch (orientation)
    {
        case kNorthernOrientation: return kSouthernOrientation;
        case kEasternOrientation: return kWesternOrientation;
        case kSouthernOrientation: return kNorthernOrientation;
        case kWesternOrientation: return kEasternOrientation;
        default: return kSouthernOrientation;
    }
}

Coord Player::orientation2Coord(uint orientation)
{
	switch (orientation)
    {
        case kNorthernOrientation: return cmc(0, -1);
        case kEasternOrientation: return cmc(1, 0);
        case kSouthernOrientation: return cmc(0, 1);
        case kWesternOrientation: return cmc(-1, 0);
        default: return CMCoordZero;
    }
}

uint Player::coord2Orientation(const Coord& moveVec)
{
	if (moveVec.x)
	{
		return moveVec.x > 0 ? kEasternOrientation : kWesternOrientation;
	}
	else if (moveVec.y)
	{
		return moveVec.y > 0 ? kSouthernOrientation : kNorthernOrientation;
	}
	else
		return kSouthernOrientation;
}

void Player::registerView(IPlayerView* view)
{
	CCAssert(!mViewsLocked, "IPlayerView is immutable during event propagation.");
	if (mViews == NULL)
		mViews = new PlayerViews();
	if (view)
		mViews->insert(view);
}

void Player::deregisterView(IPlayerView* view)
{
	CCAssert(!mViewsLocked, "IPlayerView is immutable during event propagation.");
	if (mViews && view)
		mViews->erase(view);
}

void Player::notifyPropertyChange(uint code, int value)
{
	mViewsLocked = true;
	if (mViews && mViews->size() > 0)
	{
		for(PlayerViews::iterator it = mViews->begin(); it != mViews->end(); ++it)
			(*it)->playerValueDidChange(code, value);
	}
	mViewsLocked = false;
}

void Player::notifyWillBeginMoving(void)
{
	mViewsLocked = true;
	if (mViews && mViews->size() > 0)
	{
		for(PlayerViews::iterator it = mViews->begin(); it != mViews->end(); ++it)
			(*it)->willBeginMoving();
	}
	mViewsLocked = false;
}

void Player::notifyDidFinishMoving(void)
{
	mViewsLocked = true;
	if (mViews && mViews->size() > 0)
	{
		for(PlayerViews::iterator it = mViews->begin(); it != mViews->end(); ++it)
			(*it)->didFinishMoving();
	}
	mViewsLocked = false;
}

void Player::notifyDidIdle(void)
{
    mViewsLocked = true;
	if (mViews && mViews->size() > 0)
	{
		for(PlayerViews::iterator it = mViews->begin(); it != mViews->end(); ++it)
			(*it)->didIdle();
	}
	mViewsLocked = false;
}

void Player::notifyDidTreadmill(void)
{
    mViewsLocked = true;
	if (mViews && mViews->size() > 0)
	{
		for(PlayerViews::iterator it = mViews->begin(); it != mViews->end(); ++it)
			(*it)->didTreadmill();
	}
	mViewsLocked = false;
}

void Player::rotateCW(int increments)
{
	for (int i = 0; i < increments; ++i)
		setOrientation((getOrientation() + 1) % 4);
}

void Player::rotateACW(int increments)
{
	for (int i = 0; i < increments; ++i)
		setOrientation((getOrientation() - (uint)1) % (uint)4);
}

void Player::beginMoveTo(const Coord& pos)
{
	if (isMoving())
        return;

    setIsMovingTo(pos);

	Coord moveVector = cmc(pos.x - getPosition().x, pos.y - getPosition().y);
    if (moveVector.x != 0)
        setOrientation(moveVector.x < 0 ? kWesternOrientation : kEasternOrientation);
    else if (moveVector.y != 0)
        setOrientation(moveVector.y < 0 ? kNorthernOrientation : kSouthernOrientation);

    notifyWillBeginMoving();

    if (!isMoving())
        moveTo(getIsMovingTo());
}

void Player::didBeginMoving(void)
{
	setIsMoving(true);
}

void Player::didFinishMoving(void)
{
	if (isMoving())
    {
        setIsMoving(false);
        moveTo(getIsMovingTo());
        notifyDidFinishMoving();
    }
}

void Player::idle(void)
{
    notifyDidIdle();
}

void Player::treadmill(const Coord& dir)
{
    if (dir.x != 0)
        setOrientation(dir.x < 0 ? kWesternOrientation : kEasternOrientation);
    else if (dir.y != 0)
        setOrientation(dir.y < 0 ? kNorthernOrientation : kSouthernOrientation);
    
    notifyDidTreadmill();
}

void Player::silentMoveTo(const Coord& pos)
{
	setPosition(pos);
}

void Player::moveTo(const Coord& pos)
{
	setPosition(pos);
    setDidMove(true);

    if (isColorMagicActive())
        setNumColorMagicMoves(getNumColorMagicMoves()-1);
}

void Player::teleportTo(const Coord& pos)
{
	setPosition(pos);
	notifyPropertyChange(kValueTeleported, -1);
}

void Player::reset(void)
{
	setOrientation(getDevOrientation());
    setColorKey(getDevColorKey());
    setPosition(getDevPosition());

    setPrevOrientation(getOrientation());
    setPrevColorKey(getColorKey());

    setFunction(0);
    setFunctionData(0);

    setIsColorMagicActive(false);
    setIsMoving(false);
    setDidMove(false);
    setIsMovingTo(CMCoordZero);
    setQueuedMove(CMCoordZero);
}
