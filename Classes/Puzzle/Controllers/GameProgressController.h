#ifndef __GAME_PROGRESS_CONTROLLER_H__
#define __GAME_PROGRESS_CONTROLLER_H__

#include "cocos2d.h"
#include <Puzzle/Data/GameProgressEnums.h>
class GameProgress;
USING_NS_CC;

class GameProgressController : public CCObject
{
public:
	static GameProgressController* GPC(void);
	virtual ~GameProgressController(void);

	bool hasPerformedInitialLoad(void) const { return mHasPerformedInitialLoad; }

	int getNextUnsolvedPuzzleIndex(int startIndex);
	bool isLevelUnlocked(int levelIndex);
	bool isPuzzleUnlocked(int levelIndex, int puzzleIndex);
    bool getUnlockedAll(void) const { return mUnlockedAll; };
    void setUnlockedAll(bool value) { mUnlockedAll = value; }

	// Pass-throughs
	int getNumLevels(void) const;
	int getNumSolvedLevels(void);
	int getNumPuzzles(void) const;
	int getNumPuzzlesPerLevel(void) const;
	int getNumSolvedPuzzles(void);

	bool hasPlayed(int levelIndex, int puzzleIndex);
	void setPlayed(bool played, int levelIndex, int puzzleIndex);
	bool hasSolved(int levelIndex, int puzzleIndex) const;
	void setSolved(bool solved, int levelIndex, int puzzleIndex);
	int getNumSolvedPuzzlesForLevel(int levelIndex) const;

    void invalidateCaches(void);
    const u8* exportData(ulong& size);
    UpgradeStatus upgradeToData(const u8* data, ulong size);
    
	void load(void);
	void save(void);
	void forceSave(void);

private:
	GameProgressController(void);
	GameProgressController(const GameProgressController& other);
	GameProgressController& operator=(const GameProgressController& rhs);
	void init(void);
    bool areAllPreviousLevelsUnlocked(int levelIndex);
    bool areAllPreviousPuzzlesUnlocked(int levelIndex, int puzzleIndex);

	bool mHasPerformedInitialLoad;
	bool mShouldSave;
    bool mUnlockedAll;
	GameProgress* mGameProgress;
};
#endif // __GAME_PROGRESS_CONTROLLER_H__
