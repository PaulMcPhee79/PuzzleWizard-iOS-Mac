
#include "PuzzleOrganizer.h"
#include <Puzzle/Data/Level.h>
#include <Puzzle/Data/Puzzle.h>
#include <Utils/Utils.h>
USING_NS_CC;

PuzzleOrganizer::PuzzleOrganizer(void)
	: mPuzzlesCache(NULL), mLevelsCache(NULL)
{
	mLevels = new CCArray(13);
	mLevelsDict = new CCDictionary();
	addLevel(Level::create(kOrphanedLevelID, "Orphans"));
}

PuzzleOrganizer::~PuzzleOrganizer(void)
{
	CC_SAFE_RELEASE_NULL(mPuzzlesCache);
	CC_SAFE_RELEASE_NULL(mLevelsCache);
	CC_SAFE_RELEASE_NULL(mLevels);
	CC_SAFE_RELEASE_NULL(mLevelsDict);
}

int PuzzleOrganizer::absolutePuzzleIndexForPuzzleID(int puzzleID)
{
	if (mLevels == NULL)
        return -1;

    int i = 0, puzzleIndex = -1;
    CCArray* puzzles = getPuzzles();
    if (puzzles)
    {
		CCObject* obj;
		CCARRAY_FOREACH(puzzles, obj)
		{
			Puzzle* puzzle = static_cast<Puzzle*>(obj);
			if (puzzle && puzzle->getID() == puzzleID)
			{
                puzzleIndex = i;
                break;
            }
            ++i;
		}
    }

    return puzzleIndex;
}

int PuzzleOrganizer::levelBasedPuzzleIndexForPuzzleID(int puzzleID) const
{
	if (mLevels == NULL)
        return -1;

    int puzzleIndex = -1;
	CCObject* obj;
	CCARRAY_FOREACH(mLevels, obj)
	{
        if (puzzleIndex != -1)
            break;
        
		Level* level = static_cast<Level*>(obj);
		if (level)
		{
			if (level->getID() == kOrphanedLevelID)
				continue;
			CCArray* puzzles = level->getPuzzles();
			if (puzzles)
			{
				int i = 0;
				CCObject* obj2;
				CCARRAY_FOREACH(puzzles, obj2)
				{
					Puzzle* puzzle = static_cast<Puzzle*>(obj2);
					if (puzzle && puzzle->getID() == puzzleID)
					{
						puzzleIndex = i;
						break;
					}
					++i;
				}
			}
		}
	}

    return puzzleIndex;
}

int PuzzleOrganizer::levelIndexForPuzzleID(int puzzleID) const
{
	if (mLevels == NULL)
        return -1;

    int i = 0, levelIndex = -1;
	CCObject* obj;
	CCARRAY_FOREACH(mLevels, obj)
	{
        if (levelIndex != -1)
            break;
        
		Level* level = static_cast<Level*>(obj);
		if (level)
		{
			if (level->getID() == kOrphanedLevelID)
				continue;
			CCArray* puzzles = level->getPuzzles();
			if (puzzles)
			{
				CCObject* obj2;
				CCARRAY_FOREACH(puzzles, obj2)
				{
					Puzzle* puzzle = static_cast<Puzzle*>(obj2);
					if (puzzle && puzzle->getID() == puzzleID)
					{
						levelIndex = i;
						break;
					}
				}
			}
			++i;
		}
	}

    return levelIndex;
}

Level* PuzzleOrganizer::levelForPuzzle(Puzzle* puzzle) const
{
	if (puzzle == NULL || mLevels == NULL)
        return NULL;

	CCObject* obj;
	CCARRAY_FOREACH(mLevels, obj)
	{
		Level* level = static_cast<Level*>(obj);
		if (level && level->getPuzzleForID(puzzle->getID()))
			return level;
	}

    return NULL;
}

Level* PuzzleOrganizer::levelForID(int levelID) const
{
	if (mLevelsDict)
		return static_cast<Level*>(mLevelsDict->objectForKey(levelID));
	else
		return NULL;
}

void PuzzleOrganizer::addLevel(Level* level)
{
	if (mLevels && mLevelsDict && !mLevelsDict->objectForKey(level->getID()))
    {
		mLevels->addObject(level);
		mLevelsDict->setObject(level, level->getID());
    }
}

Puzzle* PuzzleOrganizer::puzzleForID(int puzzleID, int levelID) const
{
	if (mLevels == NULL || mLevelsDict == NULL)
        return NULL;

    if (levelID == -1)
    {
		CCObject* obj;
		CCARRAY_FOREACH(mLevels, obj)
		{
			Level* level = static_cast<Level*>(obj);
			if (level)
			{
				Puzzle* puzzle = level->getPuzzleForID(puzzleID);
				if (puzzle)
					return puzzle;
			}
		}
	}
	else
	{
		Level* level = static_cast<Level*>(mLevelsDict->objectForKey(levelID));
		if (level)
			return level->getPuzzleForID(puzzleID);
	}

	return NULL;
}

void PuzzleOrganizer::addPuzzle(Puzzle* puzzle, int levelID)
{
	if (puzzle == NULL || mLevelsDict == NULL)
        return;

	if (!mLevelsDict->objectForKey(levelID))
		addLevel(Level::create(levelID, CMUtils::strConcatVal("Level", mLevels->count()).c_str()));
	if (!puzzleForID(puzzle->getID(), levelID))
		static_cast<Level*>(mLevelsDict->objectForKey(levelID))->addPuzzle(puzzle);
    else
    {
		Level* level = static_cast<Level*>(mLevelsDict->objectForKey(levelID));
		int index = level->getIndexOfPuzzleID(puzzle->getID());
		level->removePuzzle(level->getPuzzleForID(puzzle->getID()));
		level->insertPuzzleAtIndex(index, puzzle);
    }
    
    CC_SAFE_RELEASE_NULL(mPuzzlesCache);
}

void PuzzleOrganizer::loadPuzzleByID(int puzzleID)
{
	Puzzle* puzzle = puzzleForID(puzzleID);
    if (puzzle)
		dispatchEvent(EV_TYPE_PUZZLE_LOADED(), puzzle->devClone());
}

void PuzzleOrganizer::load(const char* filePath)
{
	CCAssert(mLevels, "PuzzleOrganizer: bad state in load.");

	if (!filePath)
	{
		CCLog("PuzzleOrganizer: Invalid file path. Could not load level data..");
		return;
	}

	ulong size = 0, offset = 0;
	u8* stream = CCFileUtils::sharedFileUtils()->getFileData(filePath, "rb", &size);

	if (!stream)
	{
		CCLog("PuzzleOrganizer: Could not open file data from path %s.", filePath);
		return;
	}

	while (offset < size)
	{
		Level* level = Level::createFromStream(Level::nextLevelID(), stream, &offset);
		if (level)
			this->addLevel(level);
		else
		{
			CCLog("PuzzleOrganizer: Failed to load level. File size: %lu Stream offset: %lu", size, offset);
			return;
		}
	}

	delete[] stream;
	CCLog("PuzzleOrganizer: Successfully loaded level data. File size: %lu Stream offset: %lu Level count: %u", size, offset, mLevels->count() - 1);
}

CCArray* PuzzleOrganizer::getPuzzles(void)
{
	int numPuzzles = 0;
	CCObject* obj;
	CCARRAY_FOREACH(mLevels, obj)
	{
		Level* level = static_cast<Level*>(obj);
		if (level)
		{
			if (level->getID() == kOrphanedLevelID)
				continue;
			numPuzzles += level->getNumPuzzles();
		}
	}

	if (mPuzzlesCache && (int)mPuzzlesCache->count() == numPuzzles)
        return mPuzzlesCache;

    CCArray* puzzles = new CCArray(numPuzzles);
	CCARRAY_FOREACH(mLevels, obj)
	{
		Level* level = static_cast<Level*>(obj);
		if (level)
		{
			if (level->getID() == kOrphanedLevelID)
				continue;

			CCArray* levelPuzzles = level->getPuzzles();
			if (levelPuzzles)
			{
				CCObject* obj2;
				CCARRAY_FOREACH(levelPuzzles, obj2)
				{
					Puzzle* puzzle = static_cast<Puzzle*>(obj2);
					if (puzzle)
						puzzles->addObject(puzzle);
				}
			}
		}
	}

	CC_SAFE_RELEASE(mPuzzlesCache);
    mPuzzlesCache = puzzles;
    return puzzles;
}

CCArray* PuzzleOrganizer::getLevels(void)
{
	int numLevels = 0;
	CCObject* obj;
	CCARRAY_FOREACH(mLevels, obj)
	{
		Level* level = static_cast<Level*>(obj);
		if (level)
		{
			if (level->getID() == kOrphanedLevelID)
				continue;
			++numLevels;
		}
	}

	if (mLevelsCache && (int)mLevelsCache->count() == numLevels)
        return mLevelsCache;

    CCArray* levels = new CCArray(numLevels);
	CCARRAY_FOREACH(mLevels, obj)
	{
		Level* level = static_cast<Level*>(obj);
		if (level)
		{
			if (level->getID() == kOrphanedLevelID)
				continue;
			levels->addObject(level);
		}
	}

	CC_SAFE_RELEASE(mLevelsCache);
    mLevelsCache = levels;
    return levels;
}
