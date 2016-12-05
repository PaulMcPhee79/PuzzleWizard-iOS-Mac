
#include "AnimPlayerPiece.h"
#include <Extensions/MovieClip.h>
#include <Utils/SpriteColorer.h>
#include <Utils/FloatTweener.h>
#include <Puzzle/Data/PuzzleTile.h>
#include <Puzzle/View/PuzzleHelper.h>
#include <Puzzle/View/Playfield/Effects/Twinkle.h>
#include <Utils/Utils.h>
#include <Utils/PWDebug.h>
USING_NS_CC;

const float AnimPlayerPiece::kSingleMoveDuration = 0.36f;
const float AnimPlayerPiece::kMirroredAlpha = 0.8f;
const float AnimPlayerPiece::kColorTransitionDuration = 0.35f;
const float AnimPlayerPiece::kTeleportTwinkleDuration = 0.7f;

static const float kClipOffsetY = 32.0f;

static const int kLowerIndex = 0;
static const int kMidIndex = 1;
static const int kUpperIndex = 2;
static const int kNumIndexes = 3;

AnimPlayerPiece::AnimPlayerPiece(Player* player, bool shouldPlaySounds)
	: PlayerPiece(player)
    , mIsInitializing(true)
    , mDidTeleportThisFrame(false)
    , mCurrentClipIndex(0)
    , mMagicX(0.0f)
    , mMirrorX(0.0f)
	, mSkinColoree(NULL)
	, mSkinColorer(NULL)
    , mNormalShader(NULL)
    , mStationaryLowerClips()
    , mStationaryMidClips()
    , mStationaryUpperClips()
    , mMovingLowerClips()
    , mMovingMidClips()
    , mMovingUpperClips()
{
	mAdvanceable = true;
    mShouldPlaySounds = shouldPlaySounds;
	mMoveTweener = new FloatTweener(0, this);
    mMoveTweener->setOverflows(true);
	mTwinkles = new CCArray(3);
	mColorMagicShader = CCShaderCache::sharedShaderCache()->programForKey("ColorGradient");
	mMirrorImageShader = CCShaderCache::sharedShaderCache()->programForKey("MirrorImage");
}

AnimPlayerPiece::~AnimPlayerPiece(void)
{
	if (mStationaryLowerClips)
	{
		for (int i = 0; i < kNumOrientations; ++i)
		{
			mScene->removeFromJuggler(mStationaryLowerClips[i]);
			CC_SAFE_RELEASE_NULL(mStationaryLowerClips[i]);
		}
		//delete [] mStationaryLowerClips;
		//mStationaryLowerClips = NULL;
	}
    
    if (mStationaryMidClips)
	{
		for (int i = 0; i < kNumOrientations; ++i)
		{
			mScene->removeFromJuggler(mStationaryMidClips[i]);
			CC_SAFE_RELEASE_NULL(mStationaryMidClips[i]);
		}
		//delete [] mStationaryMidClips;
		//mStationaryMidClips = NULL;
	}

	if (mStationaryUpperClips)
	{
		for (int i = 0; i < kNumOrientations; ++i)
		{
			mScene->removeFromJuggler(mStationaryUpperClips[i]);
			CC_SAFE_RELEASE_NULL(mStationaryUpperClips[i]);
		}
		//delete [] mStationaryUpperClips;
		//mStationaryUpperClips = NULL;
	}

	if (mMovingLowerClips)
	{
		for (int i = 0; i < kNumOrientations; ++i)
		{
			mScene->removeFromJuggler(mMovingLowerClips[i]);
			CC_SAFE_RELEASE_NULL(mMovingLowerClips[i]);
		}
		//delete [] mMovingLowerClips;
		//mMovingLowerClips = NULL;
	}
    
    if (mMovingMidClips)
	{
		for (int i = 0; i < kNumOrientations; ++i)
		{
			mScene->removeFromJuggler(mMovingMidClips[i]);
			CC_SAFE_RELEASE_NULL(mMovingMidClips[i]);
		}
		//delete [] mMovingMidClips;
		//mMovingMidClips = NULL;
	}

	if (mMovingUpperClips)
	{
		for (int i = 0; i < kNumOrientations; ++i)
		{
			mScene->removeFromJuggler(mMovingUpperClips[i]);
			CC_SAFE_RELEASE_NULL(mMovingUpperClips[i]);
		}
		//delete [] mMovingUpperClips;
		//mMovingUpperClips = NULL;
	}

	if (mSkinColorer)
		mScene->removeFromJuggler(mSkinColorer);
	CC_SAFE_RELEASE_NULL(mSkinColorer);
    CC_SAFE_RELEASE_NULL(mSkinColoree);

	if (mMoveTweener)
		mScene->removeFromJuggler(mMoveTweener);
	CC_SAFE_RELEASE_NULL(mMoveTweener);

	if (mTwinkles)
	{
		CCObject* obj;
		CCARRAY_FOREACH(mTwinkles, obj)
		{
			Twinkle* twinkle = static_cast<Twinkle*>(obj);
			if (twinkle)
			{
				twinkle->returnToPool();
				twinkle->setListener(NULL);
				CC_SAFE_RELEASE(twinkle);
			}
		}
	}
	CC_SAFE_RELEASE_NULL(mTwinkles);

	mColorMagicShader = NULL;
	mMirrorImageShader = NULL;
	mNormalShader = NULL;
}

bool AnimPlayerPiece::init(void)
{
	bool bRet = PlayerPiece::init();
	if (bRet)
	{
		const char* extensions[] = { "n_", "ew_", "s_", "ew_" };
        for (int i = 0; i < kNumOrientations; ++i)
        {
			for (int j = kLowerIndex; j < kNumIndexes; ++j)
			{
                const char* framesPrefix = NULL;
                MovieClip** clip = NULL;
                
                // Stationary
                switch (j)
                {
                    case kLowerIndex:
                        framesPrefix = "idle_body_";
                        clip = &mStationaryLowerClips[i];
                        break;
                    case kMidIndex:
                        framesPrefix = getIdleMidFramesPrefix();
                        clip = &mStationaryMidClips[i];
                        break;
                    case kUpperIndex:
                        framesPrefix = "idle_hat_";
                        clip = &mStationaryUpperClips[i];
                        break;
                    default:
                        CCAssert(false, "AnimPlayerPiece::init - bad Stationary clip index");
                        continue;
                }

				CCArray* frames = mScene->spriteFramesStartingWith(std::string(framesPrefix).append(extensions[i]).c_str());
				*clip = MovieClip::createWithFrames(frames, 24, false); // 42 frames (but can't get them re-exported...)
				(*clip)->setScaleX(i == 1 ? -1 : 1);
                (*clip)->setAnimatableName("AnimPlayerPiece::MovieClip::Stationary");
                
                if (i == 1)
                    (*clip)->setPositionX((*clip)->getPositionX() - 5);
                else if (i == 3)
                    (*clip)->setPositionX((*clip)->getPositionX() + 5);

                (*clip)->setPositionY(kClipOffsetY);
				(*clip)->setVisible(false);
				(*clip)->pause();
				mScene->addToJuggler(*clip);
				mPlayerCanvas->addChild(*clip);

				if (mNormalShader == NULL)
					mNormalShader = (*clip)->getShaderProgram();

                // Moving
                switch (j)
                {
                    case kLowerIndex:
                        framesPrefix = "walk_body_";
                        clip = &mMovingLowerClips[i];
                        break;
                    case kMidIndex:
                        framesPrefix = getMovingMidFramesPrefix();
                        clip = &mMovingMidClips[i];
                        break;
                    case kUpperIndex:
                        framesPrefix = "walk_hat_";
                        clip = &mMovingUpperClips[i];
                        break;
                    default:
                        CCAssert(false, "AnimPlayerPiece::init - bad Moving clip index");
                        continue;
                }
                
				frames = mScene->spriteFramesStartingWith(std::string(framesPrefix).append(extensions[i]).c_str());
                // 37.5f means we retain 24 of our 30 frames @30fps and our desired movement speed. It's a compromise forced on us by a poor art export.
                *clip = MovieClip::createWithFrames(frames, 37.5f, false);
                (*clip)->setScaleX(i == 1 ? -1 : 1);
                (*clip)->setAnimatableName("AnimPlayerPiece::MovieClip::Moving");
                
                if (i == 1)
                    (*clip)->setPositionX((*clip)->getPositionX() + 3);
                else if (i == 3)
                    (*clip)->setPositionX((*clip)->getPositionX() - 3);
                
				(*clip)->setPositionY(kClipOffsetY);
                
                if (i == 2)
                    (*clip)->setPositionY((*clip)->getPositionY() - 7);
                else if (i == 1 || i == 3)
                    (*clip)->setPositionY((*clip)->getPositionY() - 4);
                
				(*clip)->setVisible(false);
				(*clip)->pause();
				mScene->addToJuggler(*clip);
				mPlayerCanvas->addChild(*clip);
			}
        }
        
        mPlayerCanvas->setScale(1.15f);
        setCurrentClipIndex(clipIndexForPlayerOrientation(getPlayer()->getOrientation()));
        mSkinColoree = CCSprite::create(); mSkinColoree->retain();
		mSkinColorer = new SpriteColorer(mSkinColoree, this);
		syncWithData();
        
        
//#if DEBUG
//        // To reveal variable class via debugger terminal, type the following:
//        // lldb: frame variable run-target <VAR_NAME_HERE_NO_BRACKETS>
//        const int kDebugCheckerLen = 6;
//        MovieClip** debugTypeChecker[kDebugCheckerLen] =
//        {
//            mStationaryLowerClips, mStationaryMidClips, mStationaryUpperClips,
//            mMovingLowerClips, mMovingMidClips, mMovingUpperClips
//        };
//        for (int i = 0; i < kDebugCheckerLen; ++i)
//        {
//            for (int j = 0; j < kNumOrientations; ++j)
//            {
//                CCAssert(dynamic_cast<MovieClip*>(debugTypeChecker[i][j]), "INVALID MovieClip in AnimPlayerPiece::init");
//            }
//        }
//#endif
	}

	return bRet;
}

void AnimPlayerPiece::syncStationaryClipFrames()
{
    int currentFrame = getCurrentStationaryUpperClip()->getCurrentFrame();
    getCurrentStationaryLowerClip()->setCurrentFrame(currentFrame);
    getCurrentStationaryMidClip()->setCurrentFrame(currentFrame);
}

void AnimPlayerPiece::syncMovingClipFrames()
{
    int currentFrame = getCurrentMovingUpperClip()->getCurrentFrame();
    getCurrentMovingLowerClip()->setCurrentFrame(currentFrame);
    getCurrentMovingMidClip()->setCurrentFrame(currentFrame);
}

void AnimPlayerPiece::setCurrentClipIndex(int value)
{
	getCurrentStationaryLowerClip()->setVisible(false);
    getCurrentStationaryMidClip()->setVisible(false);
	getCurrentStationaryUpperClip()->setVisible(false);
	getCurrentMovingLowerClip()->setVisible(false);
    getCurrentMovingMidClip()->setVisible(false);
	getCurrentMovingUpperClip()->setVisible(false);

    mCurrentClipIndex = value;

	Player* player = getPlayer();
	if (player)
	{
		CCGLProgram* shaderProgram = player->getType() == Player::MIRRORED_MATE ? mMirrorImageShader : (player->isColorMagicActive() ? mColorMagicShader : mNormalShader);
		getCurrentStationaryLowerClip()->setShaderProgram(shaderProgram);
        getCurrentStationaryMidClip()->setShaderProgram(shaderProgram);
		getCurrentStationaryUpperClip()->setShaderProgram(shaderProgram);
		getCurrentMovingLowerClip()->setShaderProgram(shaderProgram);
        getCurrentMovingMidClip()->setShaderProgram(shaderProgram);
		getCurrentMovingUpperClip()->setShaderProgram(shaderProgram);

		if (getState() == PlayerPiece::STATIONARY)
		{
			getCurrentStationaryLowerClip()->play();
            getCurrentStationaryMidClip()->play();
			getCurrentStationaryUpperClip()->play();
            syncStationaryClipFrames();
			getCurrentStationaryLowerClip()->setVisible(true);
            getCurrentStationaryMidClip()->setVisible(true);
			getCurrentStationaryUpperClip()->setVisible(true);

			getCurrentMovingLowerClip()->pause();
            getCurrentMovingMidClip()->pause();
			getCurrentMovingUpperClip()->pause();
			getCurrentMovingLowerClip()->setVisible(false);
            getCurrentMovingMidClip()->setVisible(false);
			getCurrentMovingUpperClip()->setVisible(false);
		}
		else
		{
			getCurrentStationaryLowerClip()->pause();
            getCurrentStationaryMidClip()->pause();
			getCurrentStationaryUpperClip()->pause();
			getCurrentStationaryLowerClip()->setVisible(false);
            getCurrentStationaryMidClip()->setVisible(false);
			getCurrentStationaryUpperClip()->setVisible(false);

			getCurrentMovingLowerClip()->play();
            getCurrentMovingMidClip()->play();
			getCurrentMovingUpperClip()->play();
			//getCurrentMovingLowerClip()->setCurrentFrame(getCurrentMovingUpperClip()->getCurrentFrame());
			getCurrentMovingLowerClip()->setVisible(true);
            getCurrentMovingMidClip()->setVisible(true);
			getCurrentMovingUpperClip()->setVisible(true);
		}
	}
}

void AnimPlayerPiece::setState(PlayerPiece::PPState value)
{
	PlayerPiece::setState(value);

    switch (value)
    {
		case PlayerPiece::STATIONARY:
            {
				getCurrentStationaryLowerClip()->play();
                getCurrentStationaryMidClip()->play();
				getCurrentStationaryUpperClip()->play();
				syncStationaryClipFrames();
				getCurrentStationaryLowerClip()->setVisible(true);
                getCurrentStationaryMidClip()->setVisible(true);
				getCurrentStationaryUpperClip()->setVisible(true);

				getCurrentMovingLowerClip()->pause();
                getCurrentMovingMidClip()->pause();
				getCurrentMovingUpperClip()->pause();
				getCurrentMovingLowerClip()->setVisible(false);
                getCurrentMovingMidClip()->setVisible(false);
				getCurrentMovingUpperClip()->setVisible(false);
                
                clampToTileGrid();
            }
            break;
        case PlayerPiece::TREADMILL:
		case PlayerPiece::MOVING:
            {
				getCurrentStationaryLowerClip()->pause();
                getCurrentStationaryMidClip()->pause();
				getCurrentStationaryUpperClip()->pause();
				getCurrentStationaryLowerClip()->setVisible(false);
                getCurrentStationaryMidClip()->setVisible(false);
				getCurrentStationaryUpperClip()->setVisible(false);

				getCurrentMovingLowerClip()->play();
                getCurrentMovingMidClip()->play();
				getCurrentMovingUpperClip()->play();
				//getCurrentMovingLowerClip()->setCurrentFrame(getCurrentMovingUpperClip()->getCurrentFrame());
				getCurrentMovingLowerClip()->setVisible(true);
                getCurrentMovingMidClip()->setVisible(true);
				getCurrentMovingUpperClip()->setVisible(true);

                if (value == PlayerPiece::MOVING)
                {
                    Player* player = getPlayer();
                    if (player && player->getColorKey() != player->getFutureColorKey())
                        transitionToSkinColor(CMUtils::uint2color3B(Puzzles::playerColorForKey(player->getFutureColorKey())));
                }
            }
            break;
    }
}

float AnimPlayerPiece::getTweenMoveTo(void)
{
	if (getPlayer()->getOrientation() == Player::kEasternOrientation || getPlayer()->getOrientation() == Player::kWesternOrientation)
		return getPositionX();
	else
		return getPositionY();
}

void AnimPlayerPiece::setTweenMoveTo(float value)
{
	if (getPlayer()->getOrientation() == Player::kEasternOrientation || getPlayer()->getOrientation() == Player::kWesternOrientation)
		setPositionX(value);
	else
		setPositionY(value);
}

void AnimPlayerPiece::clampToTileGrid(void)
{
    Player* player = getPlayer();
    if (player)
    {
        Coord pos = player->getViewPosition();
        vec2 moveDims = player->getMoveDimensions();
        setPosition(ccp(pos.x * moveDims.x, pos.y * moveDims.y));
    }
}

void AnimPlayerPiece::transitionOut(float duration)
{
    PlayerPiece::transitionOut(duration);
    
    if (mNormalShader)
    {
        if (getCurrentStationaryLowerClip()->getShaderProgram() == mColorMagicShader
            || getCurrentStationaryMidClip()->getShaderProgram() == mColorMagicShader
            || getCurrentStationaryUpperClip()->getShaderProgram() == mColorMagicShader
            || getCurrentMovingLowerClip()->getShaderProgram() == mColorMagicShader
            || getCurrentMovingMidClip()->getShaderProgram() == mColorMagicShader
            || getCurrentMovingUpperClip()->getShaderProgram() == mColorMagicShader)
        {
            setAllClipShaderPrograms(mNormalShader);
        }
    }
}

void AnimPlayerPiece::enableMenuMode(bool enable)
{
	PlayerPiece::enableMenuMode(enable);

    if (mStationaryLowerClips && mStationaryMidClips && mStationaryUpperClips)
    {
        for (int i = 0; i < kNumOrientations; ++i)
		{
            mScene->removeFromJuggler(mStationaryLowerClips[i]);
            mScene->removeFromJuggler(mStationaryMidClips[i]);
            mScene->removeFromJuggler(mStationaryUpperClips[i]);
            mStationaryLowerClips[i]->setShaderProgram(mNormalShader);
            mStationaryMidClips[i]->setShaderProgram(mNormalShader);
            mStationaryUpperClips[i]->setShaderProgram(mNormalShader);
		}
    }

    if (mMovingLowerClips && mMovingMidClips && mMovingUpperClips)
    {
        for (int i = 0; i < kNumOrientations; ++i)
		{
            mScene->removeFromJuggler(mMovingLowerClips[i]);
            mScene->removeFromJuggler(mMovingMidClips[i]);
            mScene->removeFromJuggler(mMovingUpperClips[i]);
            mMovingLowerClips[i]->setShaderProgram(mNormalShader);
            mMovingMidClips[i]->setShaderProgram(mNormalShader);
            mMovingUpperClips[i]->setShaderProgram(mNormalShader);
		}
    }

    if (!enable)
    {
        if (mStationaryLowerClips && mStationaryMidClips && mStationaryUpperClips)
        {
            for (int i = 0; i < kNumOrientations; ++i)
			{
                mScene->addToJuggler(mStationaryLowerClips[i]);
                mScene->addToJuggler(mStationaryMidClips[i]);
				mScene->addToJuggler(mStationaryUpperClips[i]);
			}
        }

        if (mMovingLowerClips && mMovingMidClips && mMovingUpperClips)
        {
            for (int i = 0; i < kNumOrientations; ++i)
			{
                mScene->addToJuggler(mMovingLowerClips[i]);
                mScene->addToJuggler(mMovingMidClips[i]);
				mScene->addToJuggler(mMovingUpperClips[i]);
			}
        }
        
        syncWithData();
    }
}

void AnimPlayerPiece::setPositionAestheticOnly(const CCPoint& value)
{
	setPosition(value);
}

void AnimPlayerPiece::refreshAesthetics(void)
{
    PlayerPiece::refreshAesthetics();
    
    if (getPlayer())
        setSkinColor(CMUtils::uint2color3B(Puzzles::playerColorForKey(getPlayer()->getColorKey())));
}

void AnimPlayerPiece::setAllClipShaderPrograms(CCGLProgram* program)
{
	if (program)
	{
		getCurrentStationaryLowerClip()->setShaderProgram(program);
        getCurrentStationaryMidClip()->setShaderProgram(program);
		getCurrentStationaryUpperClip()->setShaderProgram(program);
		getCurrentMovingLowerClip()->setShaderProgram(program);
        getCurrentMovingMidClip()->setShaderProgram(program);
		getCurrentMovingUpperClip()->setShaderProgram(program);
	}
}

void AnimPlayerPiece::setSkinColor(const ccColor3B& value)
{
	if (mSkinColorer && mSkinColorer->getListener())
		mSkinColorer->setColor(value);
    else
    {
		for (int i = 0; i < kNumOrientations; ++i)
		{
           mStationaryUpperClips[i]->setColor(value);
		   mMovingUpperClips[i]->setColor(value);
		}
    }
}

void AnimPlayerPiece::transitionToSkinColor(const ccColor3B& color)
{
	if (mSkinColorer)
	{
		mScene->removeFromJuggler(mSkinColorer);
		mSkinColorer->animateColor(color, kColorTransitionDuration);
		mScene->addToJuggler(mSkinColorer);
	}
}

void AnimPlayerPiece::didTeleport()
{
    clampToTileGrid();
    
	if (mDidTeleportThisFrame)
        return;

    mDidTeleportThisFrame = true;

    if (getShouldPlaySounds())
        mScene->playSound("player-teleport");

    Twinkle* twinkle = Twinkle::getTwinkle(CCPointZero);
	twinkle->retain();
	twinkle->setListener(this);
    mPlayerCanvas->addChild(twinkle);
	mTwinkles->addObject(twinkle);
	twinkle->animate(kTeleportTwinkleDuration);
}

void AnimPlayerPiece::onEvent(int evType, void* evData)
{
	PlayerPiece::onEvent(evType, evData);

	if (evType == SpriteColorer::EV_TYPE_COLOR_DID_CHANGE())
	{
		if (mSkinColorer == NULL)
            return;

        const ccColor3B& color = mSkinColorer->getColor();
        for (int i = 0; i < kNumOrientations; ++i)
		{
			mStationaryUpperClips[i]->setColor(color);
			mMovingUpperClips[i]->setColor(color);
		}
	}
	else if (evType == FloatTweener::EV_TYPE_FLOAT_TWEENER_CHANGED())
	{
		if (getState() == PlayerPiece::MOVING && getTweenMoveTo() != mMoveTweener->getTweenedValue())
			setTweenMoveTo(mMoveTweener->getTweenedValue());
	}
	else if (evType == FloatTweener::EV_TYPE_FLOAT_TWEENER_COMPLETED())
	{
		if (mMoveTweener && getState() == PlayerPiece::MOVING)
        {
            Player* player = getPlayer();
            if (player)
                player->didFinishMoving();
            setTweenMoveTo(mMoveTweener->getTweenedValue());
        }
	}
	else if (evType == Twinkle::EV_TYPE_ANIMATION_COMPLETED())
	{
		Twinkle* twinkle = static_cast<Twinkle*>(evData);
		twinkle->setListener(NULL);
		mTwinkles->removeObject(twinkle);

		twinkle->returnToPool();
		CC_SAFE_RELEASE(twinkle);
	}
}

void AnimPlayerPiece::syncWithPlayerPiece(PlayerPiece* playerPiece)
{
	AnimPlayerPiece* animPlayerPiece = dynamic_cast<AnimPlayerPiece*>(playerPiece);
	if (animPlayerPiece)
	{
		for (int i = 0; i < kNumOrientations; ++i)
		{
			mStationaryLowerClips[i]->setCurrentFrame(animPlayerPiece->mStationaryLowerClips[i]->getCurrentFrame());
            mStationaryMidClips[i]->setCurrentFrame(animPlayerPiece->mStationaryMidClips[i]->getCurrentFrame());
			mStationaryUpperClips[i]->setCurrentFrame(animPlayerPiece->mStationaryUpperClips[i]->getCurrentFrame());
			mMovingLowerClips[i]->setCurrentFrame(animPlayerPiece->mMovingLowerClips[i]->getCurrentFrame());
            mMovingMidClips[i]->setCurrentFrame(animPlayerPiece->mMovingMidClips[i]->getCurrentFrame());
			mMovingUpperClips[i]->setCurrentFrame(animPlayerPiece->mMovingUpperClips[i]->getCurrentFrame());
		}
	}
}

void AnimPlayerPiece::syncWithData(void)
{
	PlayerPiece::syncWithData();

	Player* player = getPlayer();
    if (player == NULL)
        return;

    if (isStationary())
        clampToTileGrid();

	setCurrentClipIndex(clipIndexForPlayerOrientation(player->getOrientation()));
    
	ccColor3B transitionColor = CMUtils::uint2color3B(Puzzles::playerColorForKey(player->getColorKey()));

    if (mIsInitializing)
        setSkinColor(transitionColor);
    else if (player->getPrevColorKey() != player->getColorKey())
		transitionToSkinColor(transitionColor);

    switch (player->getFunction())
    {
		case PuzzleTile::kTFTeleport:
            didTeleport();
            break;
    }

	if (player->isColorMagicActive() && mColorMagicShader != getCurrentMovingLowerClip()->getShaderProgram())
		setAllClipShaderPrograms(mColorMagicShader);
	else if (!player->isColorMagicActive() && mColorMagicShader == getCurrentMovingLowerClip()->getShaderProgram())
		setAllClipShaderPrograms(mNormalShader);

    mIsInitializing = false;
}

void AnimPlayerPiece::moveTo(const Coord& pos)
{
	setState(PlayerPiece::MOVING);

	Player* player = getPlayer();
	if (player && mMoveTweener)
	{
		vec2 moveDims = player->getMoveDimensions();
        if (player->getOrientation() == Player::kEasternOrientation || player->getOrientation() == Player::kWesternOrientation)
			mMoveTweener->reset(getPositionX(), pos.x * moveDims.x, kSingleMoveDuration);
		else
		{
            if (MODE_8x6)
                mMoveTweener->reset(getPositionY(), (5 - pos.y) * moveDims.y, kSingleMoveDuration);
            else
                mMoveTweener->reset(getPositionY(), (7 - pos.y) * moveDims.y, kSingleMoveDuration);
		}
	}
}

void AnimPlayerPiece::didFinishMoving(void)
{
    PlayerPiece::didFinishMoving();
    mMoveTweener->forceCompletion();
}

void AnimPlayerPiece::playerValueDidChange(uint code, int value)
{
	switch (code)
    {
		case Player::kValueProperty:
            syncWithData();
            break;
        case Player::kValueColorMagic:
            break;
        case Player::kValueMirroredMate:
            break;
        case Player::kValueOrientation:
            clampToTileGrid();
            break;
		case Player::kValueTeleported:
			break;
    }
}

void AnimPlayerPiece::advanceTime(float dt)
{
	if (getState() == PlayerPiece::MOVING)
        mMoveTweener->advanceTime(dt);

    mDidTeleportThisFrame = false;
}

void AnimPlayerPiece::visit(void)
{
	Player* player = getPlayer();
	if (player)
	{
		float dt = mScene->getTimeDelta();

		mMagicX += 0.5f * dt;
		if (mMagicX > 1.0f)
			mMagicX = mMagicX - (int)mMagicX;

		mMirrorX += dt; // / 16.0f;
		if (mMirrorX > 1.0f)
			mMirrorX = mMirrorX - (int)mMirrorX;

		if (player->isColorMagicActive() && mColorMagicShader)
		{
			CCGLProgram* glProgram = mColorMagicShader;
			glProgram->use();

			glUniform2f(mScene->uniformLocationByName("u_gradientCoordCG"), mMagicX, 0);
			GLuint texId = mScene->textureByName("color-gradient")->getName();
			ccGLBindTexture2DN(mScene->textureUnitById(texId), texId);
		}
		else if (player->getType() == Player::MIRRORED_MATE && mMirrorImageShader)
		{
			CCGLProgram* glProgram = mMirrorImageShader;
			glProgram->use();

			glUniform2f(mScene->uniformLocationByName("u_gradientCoordMI"),
				(getCurrentClipIndex() == this->clipIndexForPlayerOrientation(Player::kEasternOrientation) ? -mMirrorX : mMirrorX), 0);
			GLuint texId = mScene->textureByName("mirror-gradient")->getName();
			ccGLBindTexture2DN(mScene->textureUnitById(texId), texId);
		}
	}
	
	PlayerPiece::visit();
}

int AnimPlayerPiece::clipIndexForPlayerOrientation(uint orientation)
{
	if (orientation == Player::kEasternOrientation)
        return 1;
    else if (orientation == Player::kSouthernOrientation)
        return 2;
    else if (orientation == Player::kWesternOrientation)
        return 3;
    else
        return 0;
}

/* Reusable Implementation */
void AnimPlayerPiece::reuse(void)
{
	if (getInUse())
        return;
	PlayerPiece::reuse();

	mIsInitializing = true;
    mDidTeleportThisFrame = false;
	setPosition(CCPointZero);

    for (int i = 0; i < kNumOrientations; ++i)
    {
		mStationaryLowerClips[i]->setShaderProgram(mNormalShader);
        mStationaryLowerClips[i]->setVisible(false);
        mStationaryLowerClips[i]->pause();
		mScene->addToJuggler(mStationaryLowerClips[i]);
        
        mStationaryMidClips[i]->setShaderProgram(mNormalShader);
        mStationaryMidClips[i]->setVisible(false);
        mStationaryMidClips[i]->pause();
		mScene->addToJuggler(mStationaryMidClips[i]);

		mStationaryUpperClips[i]->setShaderProgram(mNormalShader);
        mStationaryUpperClips[i]->setVisible(false);
        mStationaryUpperClips[i]->pause();
		mScene->addToJuggler(mStationaryUpperClips[i]);

		mMovingLowerClips[i]->setShaderProgram(mNormalShader);
		mMovingLowerClips[i]->setVisible(false);
        mMovingLowerClips[i]->pause();
		mScene->addToJuggler(mMovingLowerClips[i]);
        
        mMovingMidClips[i]->setShaderProgram(mNormalShader);
		mMovingMidClips[i]->setVisible(false);
        mMovingMidClips[i]->pause();
		mScene->addToJuggler(mMovingMidClips[i]);

		mMovingUpperClips[i]->setShaderProgram(mNormalShader);
		mMovingUpperClips[i]->setVisible(false);
        mMovingUpperClips[i]->pause();
		mScene->addToJuggler(mMovingUpperClips[i]);
    }
}

void AnimPlayerPiece::hibernate(void)
{
	if (!getInUse())
        return;

	if (mStationaryLowerClips)
    {
        for (int i = 0; i < kNumOrientations; ++i)
           mScene->removeFromJuggler(mStationaryLowerClips[i]);
    }
    
    if (mStationaryMidClips)
    {
        for (int i = 0; i < kNumOrientations; ++i)
            mScene->removeFromJuggler(mStationaryMidClips[i]);
    }

	if (mStationaryUpperClips)
    {
        for (int i = 0; i < kNumOrientations; ++i)
           mScene->removeFromJuggler(mStationaryUpperClips[i]);
    }

    if (mMovingLowerClips)
    {
        for (int i = 0; i < kNumOrientations; ++i)
           mScene->removeFromJuggler(mMovingLowerClips[i]);
    }
    
    if (mMovingMidClips)
    {
        for (int i = 0; i < kNumOrientations; ++i)
            mScene->removeFromJuggler(mMovingMidClips[i]);
    }

	if (mMovingUpperClips)
    {
        for (int i = 0; i < kNumOrientations; ++i)
           mScene->removeFromJuggler(mMovingUpperClips[i]);
    }
    
    if (mNormalShader)
        setAllClipShaderPrograms(mNormalShader);

    if (mSkinColorer)
        mScene->removeFromJuggler(mSkinColorer);

    if (mTwinkles)
    {
		CCObject* obj;
		CCARRAY_FOREACH(mTwinkles, obj)
		{
			Twinkle* twinkle = static_cast<Twinkle*>(obj);
			if (twinkle)
			{
				twinkle->setListener(NULL);
				twinkle->stopAnimating();
				twinkle->returnToPool();
			}
		}

		mTwinkles->removeAllObjects();
    }

	PlayerPiece::hibernate();
}
