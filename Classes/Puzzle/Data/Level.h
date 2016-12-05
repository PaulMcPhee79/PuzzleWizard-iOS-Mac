#ifndef __LEVEL_H__
#define __LEVEL_H__

#include "cocos2d.h"
class Puzzle;
USING_NS_CC;

class Level : public CCObject
{
public:
	Level(int id, const char* name);
	virtual ~Level(void);

	static Level* create(int id, const char* name, bool autorelease = true);
	static Level* createFromStream(int id, u8* stream, ulong* offset, bool autorelease = true);

	Puzzle* getPuzzleForID(int ID) const;
	int getIndexOfPuzzleID(int puzzleID) const;
	int getIndexOfPuzzle(const Puzzle* puzzle) const;
	void insertPuzzleAtIndex(int index, Puzzle* puzzle);
	void addPuzzle(Puzzle* puzzle);
	void removePuzzle(Puzzle* puzzle);
	Level* clone();

	int getID(void) const { return mID; }
	const char* getName(void) const { return mName.c_str(); }
	void setName(const char* value) { mName = value; }
	int getNumPuzzles(void) const { return mPuzzles ? mPuzzles->count() : 0; }
	CCArray* getPuzzles(void) const { return mPuzzles; }
	static int nextLevelID(int reset = -1)
	{
		static int s_nextLevelID = 1;

		if (reset != -1)
            s_nextLevelID = reset;
        return s_nextLevelID++;
	}

private:
	Level(const Level& other);
	Level& operator=(const Level& rhs);

	int mID;
	std::string mName;
	CCArray* mPuzzles;
	CCDictionary* mPuzzlesDict; // <int,Puzzle>
};
#endif // __LEVEL_H__
