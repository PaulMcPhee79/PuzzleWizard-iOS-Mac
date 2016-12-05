#ifndef __PUZZLE_CONTROLLER_H__
#define __PUZZLE_CONTROLLER_H__

#include "cocos2d.h"
#include <Events/EventDispatcher.h>
#include <Interfaces/IEventListener.h>
#include <Puzzle/Controllers/PuzzleOrganizer.h>
class SceneController;
class Puzzle;
class PuzzleBoard;
class PathFinder;
class PlayerController;
class PlayerHUD;
class SolvedAnimation;
class PuzzleRibbon;
USING_NS_CC;

class PuzzleController : public CCObject, public EventDispatcher, public IEventListener
{
public:
    static int EV_TYPE_PUZZLE_DID_BEGIN() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
	static int EV_TYPE_PUZZLE_SOLVED_ANIMATION_COMPLETED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
	static int EV_TYPE_LICENSE_LOCK_DID_TRIGGER() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }

	PuzzleController(SceneController* scene);
	virtual ~PuzzleController(void);

	bool didLevelUnlock(void) const;
	bool didPuzzleGetSolved(void) const;
	Puzzle* getPuzzle(void) const { return mPuzzle; }
	PuzzleBoard* getPuzzleBoard(void) const { return mPuzzleBoard; }
	PuzzleOrganizer* getPuzzleOrganizer(void) const { return mPuzzleOrganizer; }
	void setPuzzleBoardPosition(const CCPoint& pos);
	int getNextUnsolvedPuzzleID(void) const;
	void displaySolvedAnimation(const CCPoint& from, const CCPoint& to);
	void displayPuzzleRibbon(void);
	void hideSolvedAnimation(void);
    void hidePuzzleRibbon(void);
	void enableMenuMode(bool enable);
	void enableLicenseLock(bool enable);
    void refreshColorScheme(void);
	bool loadPuzzleByID(int puzzleID);
    bool deserializeCurrentPuzzle(void);
    bool serializeCurrentPuzzle(void);
	void resetCurrentPuzzle(void);
	void wipeCurrentPuzzleClear(void);
    void cancelEnqueuedActions(void);
	void advanceTime(float dt);
	virtual void onEvent(int evType, void* evData);
    void resolutionDidChange(void);

private:
	void setPuzzle(Puzzle* value);
	void refreshWasUnlocked(void);
	void refreshWasSolved(void);
    void beginNewPuzzle(void);
    void repositionPlayerHUD(void);

	bool mWasNextLevelUnlocked;
	bool mWasPuzzleUnsolved;
    int mLevelIndexCache;
    int mPuzzleIndexCache;

    Puzzle* mPuzzle;             // Puzzle data
    PuzzleBoard* mPuzzleBoard;   // Puzzle view
    PlayerHUD* mPlayerHUD;

    PuzzleOrganizer* mPuzzleOrganizer;

    SolvedAnimation* mSolvedAnimation;
    PuzzleRibbon* mPuzzleRibbon;

	PathFinder* mPathFinder;
	PlayerController* mPlayerController;

    SceneController* mScene;
};
#endif // __PUZZLE_CONTROLLER_H__
