#ifndef __PUZZLE_ORGANIZER_H__
#define __PUZZLE_ORGANIZER_H__

#include "cocos2d.h"
#include <Events/EventDispatcher.h>
class Level;
class Puzzle;
USING_NS_CC;

class PuzzleOrganizer : public CCObject, public EventDispatcher
{
public:
	static int EV_TYPE_PUZZLE_LOADED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }

	PuzzleOrganizer(void);
	virtual ~PuzzleOrganizer(void);

	int absolutePuzzleIndexForPuzzleID(int puzzleID);
	int levelBasedPuzzleIndexForPuzzleID(int puzzleID) const;
	int levelIndexForPuzzleID(int puzzleID) const;
	Level* levelForPuzzle(Puzzle* puzzle) const;
	Level* levelForID(int levelID) const;
	void addLevel(Level* level);
	Puzzle* puzzleForID(int puzzleID, int levelID = -1) const;
	void addPuzzle(Puzzle* puzzle, int levelID = kOrphanedLevelID);
	void loadPuzzleByID(int puzzleID);
	void load(const char* filePath);

	CCArray* getPuzzles(void); // Invalidated by adding puzzles
	CCArray* getLevels(void);

protected:

private:
	static const int kOrphanedLevelID = 0;

	CCArray* mPuzzlesCache;
	CCArray* mLevelsCache;

	CCArray* mLevels;
	CCDictionary* mLevelsDict; // <int, Level>
};
#endif // __PUZZLE_ORGANIZER_H__
