#ifndef __GAME_PROGRESS_H__
#define __GAME_PROGRESS_H__

#include "cocos2d.h"
#include <Puzzle/Data/GameProgressEnums.h>
USING_NS_CC;

class GameProgress : public CCObject
{
public:
	static const char* kFilePathExt;

	GameProgress(int numLevels, int numPuzzlesPerLevel);
	virtual ~GameProgress(void);

	int getNumLevels(void) const { return mNumLevels; }
	int getNumSolvedLevels(void);
	int getNumPuzzles(void) const;
	int getNumPuzzlesPerLevel(void) const { return mNumPuzzlesPerLevel; }
	int getNumSolvedPuzzles(void);

	bool hasPlayed(int levelIndex, int puzzleIndex);
	void setPlayed(bool played, int levelIndex, int puzzleIndex);
	bool hasSolved(int levelIndex, int puzzleIndex) const;
	void setSolved(bool solved, int levelIndex, int puzzleIndex);
	int getNumSolvedPuzzlesForLevel(int levelIndex) const;
	
	int getMinsPlayed(void) { return mMinsPlayed; }
	void setMinsPlayed(int value);

    void invalidateCaches(void);
    const u8* exportData(ulong& size);
    UpgradeStatus upgradeToData(const u8* data, ulong size);
    
	void load(void);
	bool save(void);
    
    friend void loadCallback(void* arg);

private:
    void prepareLevelDataForNumLevels(int numLevels);
    u8 valueForPuzzleIndex(int index) const;
    int resolvedIndex(int levelIndex, int puzzleIndex) const;
	bool isValidIndexes(int levelIndex, int puzzleIndex) const;
    UpgradeStatus syncWithData(const u8* data, ulong size, ulong& offset, bool merge);

    int mNumLevels;
	int mNumPuzzlesPerLevel;
    int mNumSolvedLevelsCache;
    int mNumSolvedPuzzlesCache;
    int mMinsPlayed;
    u8* mLevelData;
    u8* mExportData;
};
#endif // __GAME_PROGRESS_H__
