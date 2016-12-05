
#include "GameProgressController.h"
#include <Puzzle/Data/GameProgress.h>
#ifdef __ANDROID__
	#include <Managers/LicenseManager.h>
#endif
USING_NS_CC;

static GameProgressController *g_sharedGameProgressController = NULL;

GameProgressController* GameProgressController::GPC(void)
{
	if (!g_sharedGameProgressController)
	{
		g_sharedGameProgressController = new GameProgressController();
		g_sharedGameProgressController->init();
	}
	return g_sharedGameProgressController;
}

GameProgressController::GameProgressController(void)
	:
	mHasPerformedInitialLoad(false),
	mShouldSave(false),
    mUnlockedAll(false),
	mGameProgress(NULL)
{
	CCAssert(g_sharedGameProgressController == NULL, "Attempted to allocate a second instance of a singleton.");
}

GameProgressController::~GameProgressController(void)
{
	CC_SAFE_RELEASE_NULL(mGameProgress);
}

void GameProgressController::init(void)
{
	if (mGameProgress)
		return;
	mGameProgress = new GameProgress(12, 6);
}

int GameProgressController::getNextUnsolvedPuzzleIndex(int startIndex)
{
	int puzzleIndex = -1, numPuzzlesPerLevel = getNumPuzzlesPerLevel();
    for (int i = 0; i < getNumLevels(); ++i)
    {
        for (int j = 0; j < numPuzzlesPerLevel; ++j)
        {
            if (startIndex >= (i * numPuzzlesPerLevel + j))
                continue;

            if ((isPuzzleUnlocked(i, j) || getUnlockedAll()) && !hasSolved(i, j))
            {
                puzzleIndex = i * numPuzzlesPerLevel + j;
                return puzzleIndex;
            }
        }
    }

    for (int i = 0; i < getNumLevels(); ++i)
    {
        for (int j = 0; j < numPuzzlesPerLevel; ++j)
        {
            if ((i * numPuzzlesPerLevel + j) >= startIndex)
                return puzzleIndex;

            if ((isPuzzleUnlocked(i, j) || getUnlockedAll()) && !hasSolved(i, j))
            {
                puzzleIndex = i * numPuzzlesPerLevel + j;
                return puzzleIndex;
            }
        }
    }

    return puzzleIndex;
}

bool GameProgressController::areAllPreviousLevelsUnlocked(int levelIndex)
{
    int countMax = MIN(levelIndex, getNumLevels());
    for (int i = 0; i < countMax; ++i)
    {
        if (getNumSolvedPuzzlesForLevel(i) < 3)
            return false;
    }
    
    return true;
}

bool GameProgressController::areAllPreviousPuzzlesUnlocked(int levelIndex, int puzzleIndex)
{
    if (levelIndex < 0 || levelIndex >= getNumLevels())
        return false;
    
    int countMax = MIN(puzzleIndex, getNumPuzzlesPerLevel());
    for (int i = 0; i < countMax; ++i)
    {
        if (!hasSolved(levelIndex, i))
            return false;
    }
    
    return true;
}

bool GameProgressController::isLevelUnlocked(int levelIndex)
{
    // 1. No level locking
//    return true;
    
    // 2. All levels except first are locked
//    return levelIndex == 0;
    
	// 3. First level is always unlocked. Final level is unlocked only when every previous level is completed.
//	return levelIndex >= 0 && (levelIndex == 0 || (levelIndex < getNumLevels() - 1 && getNumSolvedPuzzlesForLevel(levelIndex - 1) >= 3)
//		|| getNumSolvedPuzzles() >= getNumPuzzles() - getNumPuzzlesPerLevel());
    
    // 4. Unlock system to manage publisher demo Unlock All God Mode.
    return levelIndex >= 0 && levelIndex < getNumLevels() &&
        (
            (levelIndex < getNumLevels() - 1 && areAllPreviousLevelsUnlocked(levelIndex)) ||
            getNumSolvedPuzzles() >= getNumPuzzles() - getNumPuzzlesPerLevel()
        );
}

bool GameProgressController::isPuzzleUnlocked(int levelIndex, int puzzleIndex)
{
    // 1. No puzzle locking.
	//return true;

	// 2. Unlock puzzle by playing previous puzzle.
	// return isLevelUnlocked(levelIndex) && (puzzleIndex == 0 || hasPlayed(levelIndex, puzzleIndex - 1));

	// 3. Unlock puzzle by solving previous puzzle.
	//return isLevelUnlocked(levelIndex) && (puzzleIndex == 0 || hasSolved(levelIndex, puzzleIndex - 1));
    
    // 4. Unlock system to manage publisher demo Unlock All God Mode.
    return isLevelUnlocked(levelIndex) &&
        (
            levelIndex == getNumLevels() - 1 ||
            hasSolved(levelIndex, puzzleIndex) ||
            areAllPreviousPuzzlesUnlocked(levelIndex, puzzleIndex)
        );
}

// Pass-throughs
int GameProgressController::getNumLevels(void) const
{
	return mGameProgress->getNumLevels();
}

int GameProgressController::getNumSolvedLevels(void)
{
	return mGameProgress->getNumSolvedLevels();
}

int GameProgressController::getNumPuzzles(void) const
{
	return mGameProgress->getNumPuzzles();
}

int GameProgressController::getNumPuzzlesPerLevel(void) const
{
	return mGameProgress->getNumPuzzlesPerLevel();
}

int GameProgressController::getNumSolvedPuzzles(void)
{
	return mGameProgress->getNumSolvedPuzzles();
}

bool GameProgressController::hasPlayed(int levelIndex, int puzzleIndex)
{
	return mGameProgress->hasPlayed(levelIndex, puzzleIndex);
}

void GameProgressController::setPlayed(bool played, int levelIndex, int puzzleIndex)
{
	if (hasPlayed(levelIndex, puzzleIndex) != played)
    {
		mGameProgress->setPlayed(played, levelIndex, puzzleIndex);
        //mShouldSave = true;
    }
}

bool GameProgressController::hasSolved(int levelIndex, int puzzleIndex) const
{
	return mGameProgress->hasSolved(levelIndex, puzzleIndex);
}

void GameProgressController::setSolved(bool solved, int levelIndex, int puzzleIndex)
{
	if (hasSolved(levelIndex, puzzleIndex) != solved)
    {
		mGameProgress->setSolved(solved, levelIndex, puzzleIndex);
        mShouldSave = true;
    }
}

int GameProgressController::getNumSolvedPuzzlesForLevel(int levelIndex) const
{
	return mGameProgress->getNumSolvedPuzzlesForLevel(levelIndex);
}

void GameProgressController::invalidateCaches(void)
{
    mGameProgress->invalidateCaches();
}

const u8* GameProgressController::exportData(ulong& size)
{
    return mGameProgress->exportData(size);
}

UpgradeStatus GameProgressController::upgradeToData(const u8* data, ulong size)
{
    UpgradeStatus status = mGameProgress->upgradeToData(data, size);
    mShouldSave = mShouldSave || status == US_LOCAL_UPGRADE || status == US_FULL_UPGRADE;
    return status;
}

void GameProgressController::load(void)
{
	mGameProgress->load();
	mHasPerformedInitialLoad = true;
	mShouldSave = false;

#ifdef __ANDROID__
	LicenseManager::LM()->setMinsPlayed(mGameProgress->getMinsPlayed());
#endif

}

void GameProgressController::save(void)
{
    if (mShouldSave)
    {
#ifdef __ANDROID__
    	mGameProgress->setMinsPlayed(LicenseManager::LM()->getMinsPlayed());
#endif
        mGameProgress->save();
        mShouldSave = false;
    }
    
//	if (mShouldSave)
//	{
//		if (mGameProgress->save())
//		{
//			mShouldSave = false;
//			CCLog("GameProgressController: Game progress saved successfully.");
//		}
//		else
//			CCLog("GameProgressController: Failed to save game progress.");
//	}
}

void GameProgressController::forceSave(void)
{
#ifdef __ANDROID__
	mGameProgress->setMinsPlayed(LicenseManager::LM()->getMinsPlayed());
#endif
	mGameProgress->save();
	mShouldSave = false;
}
