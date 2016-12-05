
#include "PuzzleController.h"
#include <SceneControllers/SceneController.h>
#include <Puzzle/Data/Puzzle.h>
#include <Puzzle/View/Playfield/PuzzleBoard.h>
#include <Puzzle/Inputs/PathFinder.h>
#include <Puzzle/Inputs/PlayerController.h>
#include <Puzzle/Data/HumanPlayer.h>
#include <Puzzle/View/Playfield/HUD/PlayerHUD.h>
#include <Puzzle/View/Playfield/Effects/SolvedAnimation.h>
#include <Puzzle/View/Playfield/Effects/PuzzleRibbon.h>
#include <Puzzle/Controllers/GameProgressController.h>
#include <Puzzle/View/Menu/LevelMenu.h>
#include <Utils/Globals.h>
#include <Utils/LangFX.h>
USING_NS_CC;

PuzzleController::PuzzleController(SceneController* scene)
	:
mScene(scene),
mPuzzle(NULL),
mPuzzleBoard(NULL),
mPathFinder(NULL),
mWasNextLevelUnlocked(false),
mWasPuzzleUnsolved(false),
mLevelIndexCache(-1),
mPuzzleIndexCache(-1)
{
	mPlayerHUD = new PlayerHUD(CMGlobals::PLAYER_HUD, CCRectMake(0, 0, LangFX::getPlayerHUDSettings().val_1, 68));
    mPlayerHUD->setScale(IS_TABLET ? 1.0f : 0.9f);
	mPlayerHUD->init();
	scene->addProp(mPlayerHUD);

	Puzzle* puzzle = Puzzle::create(Puzzle::nextPuzzleID(), "My Puzzle", MODE_8x6 ? 8 : 10, MODE_8x6 ? 6 : 8);
	
	mPuzzleBoard = PuzzleBoard::create(CMGlobals::BOARD, puzzle, false);
	mPuzzleBoard->addEventListener(PuzzleBoard::EV_TYPE_DID_TRANSITION_IN(), this);
	mPuzzleBoard->addEventListener(PuzzleBoard::EV_TYPE_DID_TRANSITION_OUT(), this);
    mPuzzleBoard->addEventListener(PuzzleBoard::EV_TYPE_BG_EXPOSURE_WILL_BEGIN(), this);
    mPuzzleBoard->addEventListener(PuzzleBoard::EV_TYPE_BG_EXPOSURE_DID_END(), this);
    
    // Hack to shed initial shield deployment lag
    //scene->muteSfx((int)(1.15 * (1.0 / CCDirector::sharedDirector()->getAnimationInterval())));
    //mPuzzleBoard->puzzleShieldDidDeploy(0);
    //mPuzzleBoard->puzzleShieldWasWithdrawn(0);
	
	mPuzzleOrganizer = new PuzzleOrganizer();
	mPuzzleOrganizer->addEventListener(PuzzleOrganizer::EV_TYPE_PUZZLE_LOADED(), this);

#ifdef CHEEKY_MOBILE
	mPathFinder = new PathFinder();
#endif

	mPlayerController = PlayerController::createPlayerController(mPuzzleBoard);
#ifdef CHEEKY_MOBILE
	mScene->registerForTouches(CMTouches::TC_PLAYFIELD, mPlayerController);
	mPlayerController->setPathFinder(mPathFinder);
	mPlayerController->addEventListener(PlayerController::EV_TYPE_PLAYER_WILL_MOVE(), this);
	mPlayerController->addEventListener(PlayerController::EV_TYPE_PLAYER_STOPPED_SHORT(), this);
    mPlayerController->addEventListener(PlayerController::EV_TYPE_PATH_NOT_FOUND(), this);
    mPlayerController->addEventListener(PlayerController::EV_TYPE_LICENSE_LOCK_DID_TRIGGER(), this);
#else
    mScene->subscribeToInputUpdates(mPlayerController);
#endif

	setPuzzle(puzzle);
    mPuzzleOrganizer->load(MODE_8x6 ? "XPlatformLevels_8x6.dat" : "XPlatformLevels_10x8.dat");

	mSolvedAnimation = SolvedAnimation::create(CMGlobals::SUB_DIALOG, this, false);

	mPuzzleRibbon = new PuzzleRibbon(CMGlobals::SUB_DIALOG);
	mPuzzleRibbon->init();
	mPuzzleRibbon->setPositionX(scene->getViewWidth() / 2);
	mScene->addProp(mPuzzleRibbon);
}

PuzzleController::~PuzzleController(void)
{
	if (mPlayerHUD)
	{
		mScene->removeProp(mPlayerHUD);
		CC_SAFE_RELEASE_NULL(mPlayerHUD);
	}

	if (mPuzzleBoard)
	{
		mScene->removeProp(mPuzzleBoard);
		mPuzzleBoard->removeEventListener(PuzzleBoard::EV_TYPE_DID_TRANSITION_IN(), this);
		mPuzzleBoard->removeEventListener(PuzzleBoard::EV_TYPE_DID_TRANSITION_OUT(), this);
        mPuzzleBoard->removeEventListener(PuzzleBoard::EV_TYPE_BG_EXPOSURE_WILL_BEGIN(), this);
        mPuzzleBoard->removeEventListener(PuzzleBoard::EV_TYPE_BG_EXPOSURE_DID_END(), this);
		CC_SAFE_RELEASE_NULL(mPuzzleBoard);
	}

	if (mSolvedAnimation)
	{
		mScene->removeProp(mSolvedAnimation);
		mSolvedAnimation->setListener(NULL);
		mSolvedAnimation->autorelease();
		mSolvedAnimation = NULL;
	}

	if (mPuzzleRibbon)
	{
		mScene->removeProp(mPuzzleRibbon);
		mPuzzleRibbon->autorelease();
		mPuzzleRibbon = NULL;
	}

	if (mPlayerController)
	{
#ifdef CHEEKY_MOBILE
		mScene->deregisterForTouches(CMTouches::TC_PLAYFIELD, mPlayerController);
        mPlayerController->removeEventListener(PlayerController::EV_TYPE_PLAYER_WILL_MOVE(), this);
		mPlayerController->removeEventListener(PlayerController::EV_TYPE_PLAYER_STOPPED_SHORT(), this);
        mPlayerController->removeEventListener(PlayerController::EV_TYPE_PATH_NOT_FOUND(), this);
        mPlayerController->removeEventListener(PlayerController::EV_TYPE_LICENSE_LOCK_DID_TRIGGER(), this);
#else
        mScene->unsubscribeToInputUpdates(mPlayerController);
#endif
		mPlayerController->autorelease(); // TouchManager may be locked.
		mPlayerController = NULL;
	}

	if (mPuzzleOrganizer)
	{
		mPuzzleOrganizer->removeEventListener(PuzzleOrganizer::EV_TYPE_PUZZLE_LOADED(), this);
		CC_SAFE_RELEASE_NULL(mPuzzleOrganizer);
	}

	CC_SAFE_RELEASE_NULL(mPathFinder);
	setPuzzle(NULL);
    mScene->removeTweensWithTarget(this);
	mScene = NULL;
}

bool PuzzleController::didLevelUnlock(void) const
{
	if (getPuzzle())
	{
		if (!mWasNextLevelUnlocked)
        {
			GameProgressController* gpc = GameProgressController::GPC();
			if (gpc->getNumSolvedPuzzles() >= gpc->getNumPuzzles() - (gpc->getNumPuzzlesPerLevel() + 1))
				return gpc->getNumSolvedPuzzles() == gpc->getNumPuzzles() - gpc->getNumPuzzlesPerLevel();
            else
                return gpc->isLevelUnlocked(getPuzzle()->getLevelIndex() + 1);
        }
	}

	return false;
}

bool PuzzleController::didPuzzleGetSolved(void) const
{
	if (getPuzzle() && mWasPuzzleUnsolved)
		return GameProgressController::GPC()->hasSolved(getPuzzle()->getLevelIndex(), getPuzzle()->getPuzzleIndex());

    return false;
}

void PuzzleController::setPuzzle(Puzzle* value)
{
	if (mPuzzle)
    {
		mPuzzle->removeEventListener(Puzzle::EV_TYPE_PLAYER_ADDED(), this);
		mPuzzle->removeEventListener(Puzzle::EV_TYPE_PLAYER_REMOVED(), this);
        mPuzzle->removeEventListener(Puzzle::EV_TYPE_UNSOLVED_PUZZLE_WILL_SOLVE(), this);

		if (mPlayerController)
			mPuzzle->deregisterView(mPlayerController);
		if (mPathFinder)
			mPathFinder->setDataProvider(NULL);
    }

	CC_SAFE_RETAIN(value);
	CC_SAFE_RELEASE(mPuzzle);
    mPuzzle = value;

    if (mPuzzleBoard)
        mPuzzleBoard->setData(value);

    if (mPuzzle)
    {
		if (mPlayerController)
		{
			mPlayerController->reset();
			mPuzzle->registerView(mPlayerController);
		}
		if (mPathFinder)
			mPathFinder->setDataProvider(mPuzzle);
		mPuzzle->addEventListener(Puzzle::EV_TYPE_PLAYER_ADDED(), this);
		mPuzzle->addEventListener(Puzzle::EV_TYPE_PLAYER_REMOVED(), this);
        mPuzzle->addEventListener(Puzzle::EV_TYPE_UNSOLVED_PUZZLE_WILL_SOLVE(), this);
        beginNewPuzzle();
    }
}

void PuzzleController::setPuzzleBoardPosition(const CCPoint& pos)
{
	if (mPuzzleBoard)
	{
        mPuzzleBoard->updateBounds(pos);
		if (mPlayerController)
			mPlayerController->updateBoardBounds();
	}
}

int PuzzleController::getNextUnsolvedPuzzleID(void) const
{
	if (getPuzzleOrganizer() == NULL || getPuzzle() == NULL)
        return -1;

    int nextUnsolvedPuzzleID = -1;
	CCArray* puzzles = getPuzzleOrganizer()->getPuzzles();
    if (puzzles)
    {
		int startIndex = getPuzzleOrganizer()->absolutePuzzleIndexForPuzzleID(getPuzzle()->getID());
        if (startIndex != -1)
        {
			int nextUnsolvedIndex = GameProgressController::GPC()->getNextUnsolvedPuzzleIndex(startIndex);
            if (nextUnsolvedIndex != -1)
            {
				if (nextUnsolvedIndex < (int)puzzles->count())
				{
					Puzzle* puzzle = static_cast<Puzzle*>(puzzles->objectAtIndex(nextUnsolvedIndex));
					if (puzzle)
						nextUnsolvedPuzzleID = puzzle->getID();
				}
            }
        }
    }
    
    return nextUnsolvedPuzzleID;
}

void PuzzleController::refreshWasUnlocked(void)
{
	if (getPuzzle())
	{
		GameProgressController* gpc = GameProgressController::GPC();
		if (gpc->getNumSolvedPuzzles() == gpc->getNumPuzzles() - (gpc->getNumPuzzlesPerLevel() + 1))
            mWasNextLevelUnlocked = false;
        else
            mWasNextLevelUnlocked = gpc->isLevelUnlocked(getPuzzle()->getLevelIndex() + 1);
	}
}

void PuzzleController::refreshWasSolved(void)
{
    if (getPuzzle())
	{
		GameProgressController* gpc = GameProgressController::GPC();
		mWasPuzzleUnsolved = !gpc->hasSolved(getPuzzle()->getLevelIndex(), getPuzzle()->getPuzzleIndex());
	}
}

void PuzzleController::beginNewPuzzle(void)
{
	refreshWasUnlocked();
    refreshWasSolved();
    dispatchEvent(EV_TYPE_PUZZLE_DID_BEGIN(), this);
}

void PuzzleController::displaySolvedAnimation(const CCPoint& from, const CCPoint& to)
{
	if (mSolvedAnimation == NULL || mSolvedAnimation->isAnimating())
		return;

	mScene->addProp(mSolvedAnimation);
	mSolvedAnimation->animate(from, to, 1.0f);
}

void PuzzleController::displayPuzzleRibbon(void)
{
	if (mPuzzleRibbon == NULL || getPuzzle() == NULL || getPuzzleOrganizer() == NULL)
        return;

	Level* level = getPuzzleOrganizer()->levelForPuzzle(getPuzzle());
    if (level)
    {
		int levelIndex = getPuzzleOrganizer()->levelIndexForPuzzleID(getPuzzle()->getID());

        if (levelIndex >= 0 && levelIndex < LevelMenu::kNumLevels)
        {
            mPuzzleRibbon->setLevelString(LevelMenu::kLevelNames[levelIndex]);
            mPuzzleRibbon->setLevelColor(LevelMenu::kLevelColors[levelIndex]);
			mPuzzleRibbon->setPuzzleString(getPuzzle()->getName());
			mPuzzleRibbon->animate(2.0f);
        }
    }
}

void PuzzleController::hideSolvedAnimation(void)
{
	if (mSolvedAnimation)
	{
		mSolvedAnimation->stopAnimating();
		mScene->removeProp(mSolvedAnimation);
	}
}

void PuzzleController::hidePuzzleRibbon(void)
{
    if (mPuzzleRibbon)
        mPuzzleRibbon->stopAnimating();
}

void PuzzleController::enableLicenseLock(bool enable)
{
	if (mPlayerController)
		mPlayerController->setLicenseLocked(enable);
}

void PuzzleController::enableMenuMode(bool enable)
{
	PuzzleBoard* board = getPuzzleBoard();
	if (board)
    {
		board->enableMenuMode(enable, false);
        board->setVisible(!enable);

		if (!enable)
            repositionPlayerHUD();
    }
}

void PuzzleController::refreshColorScheme(void)
{
    if (getPuzzleBoard())
        getPuzzleBoard()->refreshColorScheme();
}

void PuzzleController::repositionPlayerHUD(void)
{
    PuzzleBoard* board = getPuzzleBoard();
    if (board && mPlayerHUD && mPlayerHUD->getParent())
    {
        CCRect boardBounds = board->getInvertedBoardBounds();
        CCRect hudBounds = mPlayerHUD->boundingBox();
        
        mPlayerHUD->setPosition(ccp(
                                    boardBounds.origin.x + boardBounds.size.width / 2,
                                    boardBounds.origin.y + 0.55f * hudBounds.size.height));
    }
}

bool PuzzleController::loadPuzzleByID(int puzzleID)
{
    bool didPuzzleLoadSuccessfully = false;
	if (mPuzzleOrganizer)
    {
        mPuzzleIndexCache = mPuzzleOrganizer->levelBasedPuzzleIndexForPuzzleID(puzzleID);
        mLevelIndexCache = mPuzzleOrganizer->levelIndexForPuzzleID(puzzleID);
		mPuzzleOrganizer->loadPuzzleByID(puzzleID);
        didPuzzleLoadSuccessfully = getPuzzle() && getPuzzle()->getID() == puzzleID;

        if (mPuzzleRibbon)
			mPuzzleRibbon->stopAnimating();

        if (didPuzzleLoadSuccessfully)
            GameProgressController::GPC()->setPlayed(true, mLevelIndexCache, mPuzzleIndexCache);
    }
    return didPuzzleLoadSuccessfully;
}

static const char* kSerializedPuzzleFilePath = "PW_GameState.dat";
bool PuzzleController::deserializeCurrentPuzzle(void)
{
    bool didDeserialize = false;
    if (mPuzzle)
    {
        didDeserialize = mPuzzle->deserializePuzzleState(kSerializedPuzzleFilePath);
        if (didDeserialize)
            CCLog("PuzzleController::deserializeCurrentPuzzle completed successfully.");
        else
            CCLog("PuzzleController::deserializeCurrentPuzzle was not completed.");
    }
    return didDeserialize;
}

bool PuzzleController::serializeCurrentPuzzle(void)
{
    bool didSerialize = false;
    if (mPuzzle)
    {
        didSerialize = mPuzzle->serializePuzzleState(kSerializedPuzzleFilePath);
        if (didSerialize)
            CCLog("PuzzleController::serializeCurrentPuzzle completed successfully.");
        else
            CCLog("PuzzleController::serializeCurrentPuzzle was not completed.");
    }
    return didSerialize;
}

void PuzzleController::resetCurrentPuzzle(void)
{
	if (getPuzzleBoard() == NULL || getPuzzleBoard()->isTransitioning())
        return;

	if (mPlayerController)
    {
		mPlayerController->enable(false);
        mPlayerController->reset();
    }

    PuzzleBoard* board = PuzzleBoard::create(CMGlobals::BOARD, getPuzzle(), false);
	board->setPosition(getPuzzleBoard()->getPosition());
	board->addEventListener(PuzzleBoard::EV_TYPE_DID_TRANSITION_OUT(), this);
	mScene->addToJuggler(board);
	mScene->addProp(board);

	board->transitionOut(0.5f, 0.1f, 0.1f / 2);
	board->setData(NULL);

	getPuzzleBoard()->reset(true);
	getPuzzleBoard()->transitionIn(0.5f, 0.1f, 0.1f / 2);
    beginNewPuzzle();
}

void PuzzleController::wipeCurrentPuzzleClear(void)
{
	if (mPuzzle)
		mPuzzle->wipeClear();
}

void PuzzleController::cancelEnqueuedActions(void)
{
    if (mPlayerController)
        mPlayerController->reset();
    
    hidePuzzleRibbon();
    hideSolvedAnimation();
}

void PuzzleController::advanceTime(float dt)
{
	if (mPuzzleBoard)
		mPuzzleBoard->advanceTime(dt);
	if (mPuzzle)
		mPuzzle->processPrevMovements();
	if (mPlayerController)
		mPlayerController->advanceTime(dt);
	if (mPuzzle)
		mPuzzle->processNextMovements();
	if (mPuzzleBoard)
		mPuzzleBoard->postAdvanceTime(dt);
}

void PuzzleController::onEvent(int evType, void* evData)
{
	if (evType == PlayerController::EV_TYPE_PLAYER_WILL_MOVE())
	{
		if (mPlayerController && mPuzzleBoard)
			mPuzzleBoard->enablePath(mPlayerController->getPlayer(), mPlayerController->getPath(), mPlayerController->getPathLength());
	}
	else if (evType == PlayerController::EV_TYPE_PLAYER_STOPPED_SHORT())
	{
		if (mPuzzleBoard)
			mPuzzleBoard->disablePath();
	}
    else if (evType == PlayerController::EV_TYPE_PATH_NOT_FOUND())
    {
        if (mPuzzleBoard && evData)
        {
            mPuzzleBoard->displayPathNotFound(*(int*)evData);
            mScene->playSound("error");
        }
    }
	else if (evType == Puzzle::EV_TYPE_PLAYER_ADDED())
	{
		Player* player = static_cast<Player*>(evData);
		if (player)
		{
			if (player->getType() == Player::HUMAN_PLAYER && mPlayerController && mPlayerController->getPlayer() == NULL)
				mPlayerController->setPlayer(player);

			if (mPlayerHUD)
				mPlayerHUD->setPlayer(player);
			player->broadcastProperties();
		}
	}
	else if (evType == Puzzle::EV_TYPE_PLAYER_REMOVED())
	{
		Player* player = static_cast<Player*>(evData);
		if (player)
		{
			if (mPlayerController && mPlayerController->getPlayer() == player)
				mPlayerController->setPlayer(NULL);
			if (mPlayerHUD && mPlayerHUD->getPlayer() == player)
				mPlayerHUD->setPlayer(NULL);
		}
	}
    else if (evType == Puzzle::EV_TYPE_UNSOLVED_PUZZLE_WILL_SOLVE())
    {
        dispatchEvent(evType, evData);
    }
	else if (evType == PuzzleOrganizer::EV_TYPE_PUZZLE_LOADED())
	{
		if (mPlayerHUD)
			mPlayerHUD->setPlayer(NULL);
		
        Puzzle* puzzle = static_cast<Puzzle*>(evData);
		puzzle->setPuzzleIndex(mPuzzleIndexCache);
		puzzle->setLevelIndex(mLevelIndexCache);
        setPuzzle(puzzle);

		if (mPlayerController)
		{
			mPlayerController->reset();
			Player* player = puzzle->getAnyHumanPlayer();
			if (player)
			{
				mPlayerController->setPlayer(player);

				if (mPlayerHUD)
					mPlayerHUD->setPlayer(player);
				player->broadcastProperties();
			}
		}
	}
	else if (evType == SolvedAnimation::EV_TYPE_ANIMATION_COMPLETED())
	{
		if (mSolvedAnimation)
			mScene->removeProp(mSolvedAnimation);
		dispatchEvent(EV_TYPE_PUZZLE_SOLVED_ANIMATION_COMPLETED(), this);
	}
	else if (evType == PuzzleBoard::EV_TYPE_DID_TRANSITION_IN())
	{
		if (mPlayerController)
			mPlayerController->enable(true);
        dispatchEvent(evType, evData);
	}
	else if (evType == PuzzleBoard::EV_TYPE_DID_TRANSITION_OUT())
	{
		PuzzleBoard* board = static_cast<PuzzleBoard*>(evData);
		if (board)
		{
			board->removeEventListener(PuzzleBoard::EV_TYPE_DID_TRANSITION_IN(), this);
			board->removeEventListener(PuzzleBoard::EV_TYPE_DID_TRANSITION_OUT(), this);
            board->removeEventListener(PuzzleBoard::EV_TYPE_BG_EXPOSURE_WILL_BEGIN(), this);
            board->removeEventListener(PuzzleBoard::EV_TYPE_BG_EXPOSURE_DID_END(), this);
			mScene->removeFromJuggler(board);
			mScene->removeProp(board);
			board->autorelease();
		}
	}
    else if (evType == PuzzleBoard::EV_TYPE_BG_EXPOSURE_WILL_BEGIN())
    {
        dispatchEvent(evType, evData);
    }
    else if (evType == PuzzleBoard::EV_TYPE_BG_EXPOSURE_DID_END())
    {
        dispatchEvent(evType, evData);
    }
	else if (evType == PlayerController::EV_TYPE_LICENSE_LOCK_DID_TRIGGER())
	{
		dispatchEvent(PuzzleController::EV_TYPE_LICENSE_LOCK_DID_TRIGGER(), this);
	}
}

void PuzzleController::resolutionDidChange(void)
{
    repositionPlayerHUD();
    if (mPuzzleRibbon)
        mPuzzleRibbon->resolutionDidChange();
}
