
#include "Level.h"
#include <Puzzle/Data/Puzzle.h>
#include <Utils/CMFileTypes.h>
USING_NS_CC;

Level::Level(int id, const char* name)
{
	mID = id;
	mName = name;
	mPuzzles = new CCArray(6);
	mPuzzlesDict = new CCDictionary();
}

Level::~Level(void)
{
	CC_SAFE_RELEASE(mPuzzles); mPuzzles = NULL;
	CC_SAFE_RELEASE(mPuzzlesDict); mPuzzlesDict = NULL;
}

Level* Level::create(int id, const char* name, bool autorelease)
{
	Level* level = new Level(id, name);
    if (level)
    {
		if (autorelease)
			level->autorelease();
        return level;
    }
    CC_SAFE_DELETE(level);
    return NULL;
}

Level* Level::createFromStream(int id, u8* stream, ulong* offset, bool autorelease)
{
	sXLevel* sxlevel = (sXLevel*)(stream + *offset);
	Level* level = Level::create(id, sxlevel->name, autorelease);
	
	if (level)
	{
		*offset += sizeof(*sxlevel);
		for (int i = 0; i < sxlevel->numPuzzles; ++i)
		{
			Puzzle* puzzle = Puzzle::createFromStream(Puzzle::nextPuzzleID(), stream, offset);
			if (puzzle)
				level->addPuzzle(puzzle);
		}
	}
	
	return level;
}

Puzzle* Level::getPuzzleForID(int ID) const
{
	if (mPuzzlesDict)
		return (Puzzle*)mPuzzlesDict->objectForKey(ID);
	else
		return NULL;
}

int Level::getIndexOfPuzzleID(int puzzleID) const
{
	if (mPuzzles && mPuzzlesDict)
	{
		CCObject* obj = mPuzzlesDict->objectForKey(puzzleID);
		if (obj)
			return mPuzzles->indexOfObject(obj);
	}
	
	return -1;
}

int Level::getIndexOfPuzzle(const Puzzle* puzzle) const
{
	if (mPuzzles)
		return mPuzzles->indexOfObject(const_cast<Puzzle*>(puzzle));
	else
		return -1;
}

void Level::insertPuzzleAtIndex(int index, Puzzle* puzzle)
{
	CCAssert(mPuzzles && index >= 0 && index <= (int)mPuzzles->count(), "Level::insertPuzzleAtIndex - index out of range.");

	if (puzzle && mPuzzles && index >= 0 && index <= (int)mPuzzles->count())
	{
		mPuzzles->insertObject(puzzle, (uint)index);
		if (mPuzzlesDict)
			mPuzzlesDict->setObject(puzzle, puzzle->getID());
	}
}

void Level::addPuzzle(Puzzle* puzzle)
{
	if (puzzle && mPuzzles)
		insertPuzzleAtIndex((int)mPuzzles->count(), puzzle);
}

void Level::removePuzzle(Puzzle* puzzle)
{
	if (puzzle && mPuzzles && mPuzzles->containsObject(puzzle))
    {
		mPuzzles->removeObject(puzzle);
        if (mPuzzlesDict)
			mPuzzlesDict->removeObjectForKey(puzzle->getID());
    }
}

Level* Level::clone()
{
	Level* level = new Level(getID(), getName());
	level->autorelease();

	if (mPuzzles && mPuzzles->count() > 0)
	{
		CCObject* obj;
		CCARRAY_FOREACH(mPuzzles, obj)
		{
			Puzzle* puzzle = static_cast<Puzzle*>(obj);
			if (puzzle)
				level->addPuzzle(puzzle->clone());
		}
	}

    return level;
}
