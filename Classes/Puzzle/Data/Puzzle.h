#ifndef __PUZZLE_H__
#define __PUZZLE_H__

#include "cocos2d.h"
#include <Utils/CMTypes.h>
#include <Events/EventDispatcher.h>
#include <Puzzle/Data/PuzzleTile.h>
#include <Interfaces/IPuzzleView.h>
#include <Puzzle/Data/Player.h>
class ColorFiller;
class HumanPlayer;
class MirroredPlayer;
USING_NS_CC;

class Puzzle : public CCObject, public EventDispatcher
{
public:
	static int EV_TYPE_PLAYER_ADDED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
	static int EV_TYPE_PLAYER_REMOVED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
    static int EV_TYPE_UNSOLVED_PUZZLE_WILL_SOLVE() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
    static int EV_TYPE_FORCE_COMPLETE_ALL_MECHANICS() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }

    Puzzle(int id, const char* name, int numColumns, int numRows);
	static Puzzle* create(int id, const char* name, int numColumns, int numRows, bool autorelease = true);

	virtual ~Puzzle(void);

	static Puzzle* createFromStream(int id, u8* stream, ulong* offset, bool autorelease = true);

	static int nextPuzzleID()
	{
		static int s_nextPuzzleID = 1;
		return s_nextPuzzleID++;
	}

	bool isValidIndex(int index) const { return index >= 0 && index < getNumTiles(); }
	bool isPerimeterIndex(int index) const
	{
		int row = rowForIndex(index), column = columnForIndex(index);
		return row == 0 || row == getNumRows() - 1 || column == 0 || column == getNumColumns() - 1;
	}
	bool isValidPos(const Coord& pos) const { return pos.x >= 0 && pos.x < getNumColumns() && pos.y >= 0 && pos.y < getNumRows(); }
	int pos2Index(const Coord& pos) const { return isValidPos(pos) ? pos.x + pos.y * getNumColumns() : -1; }
	Coord index2Pos(int index) const { return cmc(index % getNumColumns(), index / getNumColumns()); }
	bool doesAdjacentPerpIndexWrap(int index, int adjacentIndex) const
	{
		return (index % getNumColumns() == 0 && adjacentIndex == index - 1) || (adjacentIndex % getNumColumns() == 0 && adjacentIndex == index + 1);
	}
	bool isModified(uint modifier, int originIndex, int numColumns, int numRows) const;
	bool isIndexAxiallyAdjacent(int index, int other) const;
	bool isIndexSurroundedBy(int index, int other) const;
    bool isResetting(void) const { return mResetting; }
    bool isSolved(void) const { return mSolved; }
	Player* playerAtIndex(int index) const;
	HumanPlayer* getAnyHumanPlayer(void) const;
	void addPlayer(Player* player);
	void removePlayer(Player* player);
	void removePlayerAtIndex(int index);
	int rowForIndex(int index) const;
	int columnForIndex(int index) const;
	int indexForTile(const PuzzleTile* tile) const;
	PuzzleTile* tileAtIndex(int index) const;
	CCArray* tileRange(int originIndex, int numColumns, int numRows);
	bool isPlayerOccupied(const Player* player);
	void applyRotation(void);
	void applyTileSwap(void);
	void applyConveyorBelt(void);
	void tryMove(Player* player);
	void move(Player* player, const Coord& pos);
    void treadmill(Player* player, const Coord& dir);
	void registerView(IPuzzleView* view);
	void deregisterView(IPuzzleView* view);
	void processPrevMovements(void);
	void processNextMovements(void);
	void pause(void);
	void resume(void);
	void reset(void);
	void wipeClear(void);
	Puzzle* clone(void) const;
	Puzzle* devClone(void) const;

	virtual void puzzleSoundShouldPlay(const char* soundName);
	virtual void puzzlePlayerWillMove(Player* player);
	virtual void puzzlePlayerDidMove(Player* player);
    virtual void puzzleShieldDidDeploy(int tileIndex);
    virtual void puzzleShieldWasWithdrawn(int tileIndex);
	virtual void puzzleTilesShouldRotate(const int2d& tileIndexes);
    virtual void puzzleTileSwapWillBegin(const int2d& swapIndexes, bool isCenterValid);
	virtual void puzzleConveyorBeltWillMove(const Coord& moveDir, int wrapIndex, const int1d* tileIndexes, int numTiles);
    virtual void puzzleWasSolved(int tileIndex);

    void enableSearchWeighting(bool enable);
	bool isRotating(void) const { return mIsRotating; }
	bool isConveyorBeltActive(void) const { return mIsConveyorBeltActive; }
	bool isTileSwapping(void) const { return mIsTileSwapping; }
	int getID(void) const { return mID; }
    int getIQ(void) const { return mIQ; }
	int getPuzzleIndex(void) const { return mPuzzleIndex; }
	void setPuzzleIndex(int value) { mPuzzleIndex = value; }
	int getLevelIndex(void) const { return mLevelIndex; }
	void setLevelIndex(int value) { mLevelIndex = value; }
	const char* getName(void) const { return mName.c_str(); }
	void setName(const char* value) { mName = value; }
	int getNumTiles(void) const { return getNumColumns() * getNumRows(); }
	int getNumColumns(void) const { return mNumColumns; }
	int getNumRows() const { return mNumRows; }
	PuzzleTile* getTiles(void) const { return mTiles; }
	CCArray* getPlayers(void) const { return mPlayers; }
	Player* getPlayerForID(int id) const;
	const int2d& getRotationIndexes(void) { return mRotationIndexes; }
	const int2d& getTileSwapIndexes(void) { return mTileSwapIndexes; }
	int getShieldIndex(void) const { return mShieldIndex; }
	const Coord& getConveyorBeltDir(void) const { return mConveyorBeltDir; }
	int getConveyorBeltWrapIndex(void) const { return mConveyorBeltWrapIndex; }
	int1d* getConveyorBeltIndexes(void)
	{
		if (mConveyorBeltDir.x != 0)
            return &mHorizConveyorBeltIndexes;
        else if (mConveyorBeltDir.y != 0)
            return &mVertConveyorBeltIndexes;
        else
            return NULL;
	}
	int getConveyorBeltIndexCount(void)
	{
		int1d* indexes = getConveyorBeltIndexes();
		if (indexes)
			return indexes->size();
		else
			return 0;
	}
	int getSearchWeighting(int index);
    
    bool shouldSerialize(void) const { return mShouldSerialize; }
    bool deserializePuzzleState(const char* filepath);
    bool serializePuzzleState(const char* filepath);
    std::string getCurrentPuzzleSerialized();
    friend void deserializeCallback(void* arg);
    friend void serializeCallback(void* arg);

private:
    enum SERIALIZE_STATE { SERIALIZE_NONE = 0, SERIALIZE_LOAD, SERIALIZE_SAVE };
    
	static const int* kPaintBombIndexOffsets;
	static const int kQueuedCommandsCapacity = 10;
    static unsigned char* serializeBuffer;

	PuzzleTile* tileForTile(const PuzzleTile* tile) const;
	void enableShield(int index, bool enable);
	void refreshBoardFunction(uint function);
	void recolorPlayers(void);
	int getCommandPriority(uint command);
	void addQueuedCommand(Player* player);
	void playerMoved(Player* player);
	void processQueuedCommands(void);
	void resetTileSwapIndexes(void);

    void setIQ(int value) { mIQ = value; }
	void setIsRotating(bool value) { mIsRotating = value; }
	void setIsConveyorBeltActive(bool value) { mIsConveyorBeltActive = value; }
	void setIsTileSwapping(bool value) { mIsTileSwapping = value; }
	CCArray* getHumanPlayers(void) const;
	void setShieldIndex(int value) { mShieldIndex = value; }
	void setConveyorBeltDir(const Coord& value) { mConveyorBeltDir = value; }
	void setConveyorBeltWrapIndex(int value) { mConveyorBeltWrapIndex = value; }
    SERIALIZE_STATE getSerializeState(void) const { return mSerializeState; }
    void setSerializeState(SERIALIZE_STATE value)
    {
        if (value == SERIALIZE_LOAD || value == SERIALIZE_SAVE)
            mWasSerializationSuccessful = false;
        if (value == SERIALIZE_SAVE)
            mSerializeWriteCount = 0;
        mSerializeState = value;
    }

	bool mSolved;
    bool mIsRotating;
    bool mIsTileSwapping;
    bool mIsConveyorBeltActive;
    bool mPaused;
    bool mResetting;
    bool mIsSearchWeightingEnabled;
    bool mShouldSerialize;
    bool mWasSerializationSuccessful;
    int mID;
    int mIQ;
    int mLevelIndex;
    int mPuzzleIndex;
    std::string mName;
    int mNumColumns;
    int mNumRows;
    int mSerializeWriteCount;
    SERIALIZE_STATE mSerializeState;
    PuzzleTile* mTiles;
    CCArray* mPlayers;
    Player* mQueuedCommands[kQueuedCommandsCapacity];
    int mShieldIndex;
    int2d mRotationIndexes; // 2x9
    int2d mTileSwapIndexes; // 2x9

    Coord mConveyorBeltDir;
    int mConveyorBeltWrapIndex;
	int1d mVertConveyorBeltIndexes;
	int1d mHorizConveyorBeltIndexes;

	ColorFiller* mColorFiller;

	bool mViewsLocked;
    std::set<IPuzzleView*>* mViews;

	std::map<uint, int> mCommandPriorities;
	std::map<uint, int> mSearchWeightings;
	static std::map<uint, int> createCommandPriorities(void);
	static std::map<uint, int> createSearchWeightings(void);
};
#endif // __PUZZLE_H__
