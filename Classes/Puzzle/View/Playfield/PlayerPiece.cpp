
#include "PlayerPiece.h"
#include "HumanPlayerPiece.h"
#include "MirrorPlayerPiece.h"
#include <Utils/ReusableCache.h>
#include <Utils/ByteTweener.h>
#include <Puzzle/Data/HumanPlayer.h>
#include <Puzzle/View/PuzzleHelper.h>
#include <Puzzle/View/Playfield/AnimPlayerPiece.h>
#include <Puzzle/View/Playfield/StaticPlayerPiece.h>
USING_NS_CC;

PlayerPiece::PlayerPiece(Player* player)
	:
	mTransitionInTweener(NULL),
    mTransitionOutTweener(NULL),
	mPlayer(NULL),
    mPlayerCanvas(NULL),
	mState(STATIONARY)
{
	CCAssert(player, "PlayerPiece requires non-null player.");
    setPlayer(player);
}

PlayerPiece::~PlayerPiece(void)
{
	if (mPlayer)
		mPlayer->deregisterView(this);
	CC_SAFE_RELEASE_NULL(mPlayer);

	if (mTransitionInTweener)
	{
		mTransitionInTweener->setListener(NULL);
		mScene->removeFromJuggler(mTransitionInTweener);
		CC_SAFE_RELEASE_NULL(mTransitionInTweener);
	}
    
    if (mTransitionOutTweener)
	{
		mTransitionOutTweener->setListener(NULL);
		mScene->removeFromJuggler(mTransitionOutTweener);
		CC_SAFE_RELEASE_NULL(mTransitionOutTweener);
	}
    
    CC_SAFE_RELEASE_NULL(mPlayerCanvas);
}

// Derived classes should syncWithData() when ready.
bool PlayerPiece::init(void)
{
    if (mPlayerCanvas)
        return true;
    
	bool bRet = Prop::init();
    if (bRet)
    {
        mPlayerCanvas = CCNode::create();
		if (mPlayerCanvas)
        {
            mPlayerCanvas->retain();
            addChild(mPlayerCanvas);
        }
        else
            bRet = false;
    }
    
    return bRet;
}

void PlayerPiece::setPlayer(Player* value)
{
	if (mPlayer == value)
		return;
	if (mPlayer)
		mPlayer->deregisterView(this);
	CC_SAFE_RETAIN(value);
	CC_SAFE_RELEASE(mPlayer);
    mPlayer = value;
	if (mPlayer)
		mPlayer->registerView(this);
}

void PlayerPiece::transitionIn(float duration, float delay)
{
	this->setVisible(true);

	if (mTransitionInTweener == NULL)
		mTransitionInTweener = new ByteTweener(0, this);
	else
		mScene->removeFromJuggler(mTransitionInTweener);

	setOpacityChildren(0);
	mTransitionInTweener->reset(0, 255, duration, delay);
	mScene->addToJuggler(mTransitionInTweener);
}

void PlayerPiece::transitionOut(float duration)
{
    if (duration > 0)
    {
        if (mTransitionOutTweener == NULL)
            mTransitionOutTweener = new ByteTweener(0, this);
        else
            mScene->removeFromJuggler(mTransitionOutTweener);
	
        mTransitionOutTweener->reset(getOpacity(), 0, duration);
        mScene->addToJuggler(mTransitionOutTweener);
    }
    else
        dispatchEvent(EV_TYPE_DID_TRANSITION_OUT(), this);
}

void PlayerPiece::onEvent(int evType, void* evData)
{
	if (evType == ByteTweener::EV_TYPE_BYTE_TWEENER_CHANGED())
	{
		ByteTweener* tweener = static_cast<ByteTweener*>(evData);
		if (tweener && (tweener == mTransitionInTweener || tweener == mTransitionOutTweener))
			setOpacityChildren(tweener->getTweenedValue());
	}
	else if (evType == ByteTweener::EV_TYPE_BYTE_TWEENER_COMPLETED())
	{
        ByteTweener* tweener = static_cast<ByteTweener*>(evData);
        if (tweener && (tweener == mTransitionOutTweener))
            dispatchEvent(EV_TYPE_DID_TRANSITION_OUT(), this);
	}
}

void PlayerPiece::setData(Player* player)
{
    setPlayer(player);
    if (mPlayer)
		syncWithData();
}

void PlayerPiece::syncWithData(void) { /* empty implementation */ }

void PlayerPiece::moveTo(const Coord& pos)
{
	if (mPlayer)
		mPlayer->didFinishMoving();
}

void PlayerPiece::willBeginMoving(void)
{
	if (mPlayer)
    {
        mPlayer->didBeginMoving();
        moveTo(mPlayer->getIsMovingTo());
    }
}

void PlayerPiece::didIdle(void)
{
    if (getState() != PlayerPiece::STATIONARY)
        setState(PlayerPiece::STATIONARY);
}

void PlayerPiece::didTreadmill(void)
{
    if (getState() != PlayerPiece::TREADMILL)
        setState(PlayerPiece::TREADMILL);
}

void PlayerPiece::reset()
{
	setState(STATIONARY);
}

float PlayerPiece::rotationForPlayerOrientation(uint orientation)
{
	if (orientation == Player::kEasternOrientation)
        return 90;
    else if (orientation == Player::kSouthernOrientation)
        return 180;
    else if (orientation == Player::kWesternOrientation)
        return 270;
    else
        return 0;
}

/* Reusable Implementation */
ReusableCache* PlayerPiece::sCache = NULL;
bool PlayerPiece::sCaching = false;

void PlayerPiece::setupReusables(void)
{
	if (sCache)
        return;

    sCaching = true;
    sCache = new ReusableCache();

	const int kNumCaches = 2;
    int cacheSizes[kNumCaches] = { 16, 12 }; // { 32, 16 };  // Human, Mirror
    
    PPType reuseKeys[kNumCaches] = { HUMAN, MIRROR };
    IReusable* reusable = NULL;
	Player* player = new HumanPlayer(Puzzles::playerColorForIndex(Puzzles::kRedIndex)); // Doesn't matter that we re-use for MIRROR here
	player->autorelease();

    for (int i = 0; i < kNumCaches; ++i)
    {
        int cacheSize = cacheSizes[i];
        sCache->addKey(cacheSize, reuseKeys[i]);

        for (int j = 0; j < cacheSize; ++j)
        {
            reusable = getPlayerPiece(player, reuseKeys[i]);
            reusable->hibernate();
            sCache->addReusable(reusable);
        }
    }
    
    sCache->verifyCacheIntegrity();
    sCaching = false;
}

IReusable* PlayerPiece::checkoutReusable(uint reuseKey)
{
	IReusable* reusable = NULL;

    if (sCache && !sCaching)
		reusable = sCache->checkout(reuseKey);

    return reusable;
}

void PlayerPiece::checkinReusable(IReusable* reusable)
{
	if (sCache && !sCaching && reusable)
		sCache->checkin(reusable);
}

PlayerPiece* PlayerPiece::getPlayerPiece(Player* player, bool shouldPlaySounds)
{
    if (player) {
        PPType type;
        
        switch (player->getType())
        {
            case Player::HUMAN_PLAYER: type = HUMAN; break;
            case Player::MIRRORED_MATE: type = MIRROR; break;
            default: type = HUMAN; break;
        }
        
        return getPlayerPiece(player, type, shouldPlaySounds);
    }
    else
    {
        return NULL;
    }
}

PlayerPiece* PlayerPiece::getPlayerPiece(Player* player, PPType type, bool shouldPlaySounds)
{
	int reuseKey = (int)type;
    PlayerPiece* playerPiece = static_cast<PlayerPiece*>(checkoutReusable(reuseKey));

    if (playerPiece)
    {
        playerPiece->reuse();
        playerPiece->setData(player);

        switch (type)
        {
			case PlayerPiece::HUMAN:
            case PlayerPiece::MIRROR:
                {
                    AnimPlayerPiece* animPlayerPiece = static_cast<AnimPlayerPiece*>(playerPiece);
                    if (animPlayerPiece)
                        animPlayerPiece->setShouldPlaySounds(shouldPlaySounds);
                }
                break;
            default:
                break;
        }
    }
    else
    {
        switch (type)
        {
			case PlayerPiece::STATIC:
                playerPiece = StaticPlayerPiece::create(player, !sCaching);
                break;
			case PlayerPiece::HUMAN:
				playerPiece = HumanPlayerPiece::create(player, shouldPlaySounds, !sCaching);
                break;
            case PlayerPiece::MIRROR:
				playerPiece = MirrorPlayerPiece::create(player, shouldPlaySounds, !sCaching);
                break;
            default:
                CCAssert(false, "Invalid PlayerPiece requested in PlayerPiece::getPlayerPiece.");
				break;
        }
    }

    return playerPiece;
}

void PlayerPiece::reuse(void)
{
	if (getInUse())
        return;

	setPosition(ccp(0, 0));
	setScale(1);
	setRotation(0);
	setOpacityChildren(255);
	setState(STATIONARY);
	setVisible(true);
    
    mInUse = true;
}

void PlayerPiece::hibernate(void)
{
	if (!getInUse())
        return;

	if (mTransitionInTweener)
		mScene->removeFromJuggler(mTransitionInTweener);
    if (mTransitionOutTweener)
		mScene->removeFromJuggler(mTransitionOutTweener);
	mScene->removeTweensWithTarget(static_cast<IEventListener*>(this));
    removeFromParent();
    setData(NULL);

    mInUse = false;
    checkinReusable(this);
}

void PlayerPiece::returnToPool(void)
{
    if (getPoolIndex() != -1)
        hibernate();
    else
        removeFromParent();
}
