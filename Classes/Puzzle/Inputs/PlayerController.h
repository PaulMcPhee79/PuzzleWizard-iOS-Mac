
#ifndef __PLAYER_CONTROLLER_H__
#define __PLAYER_CONTROLLER_H__

#include "cocos2d.h"
#include <Events/EventDispatcher.h>
#include <Interfaces/IEventListener.h>
#include <Interfaces/IPuzzleView.h>
#include <Interfaces/IPlayerView.h>
#include <Interfaces/IInteractable.h>
#include <Utils/CMTypes.h>
class Player;
class PuzzleBoard;
class PathFinder;
USING_NS_CC;

class PlayerController : public CCObject, public EventDispatcher, public IEventListener, public IPuzzleView, public IPlayerView, public IInteractable
{
public:
    static int EV_TYPE_PLAYER_WILL_MOVE() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
	static int EV_TYPE_PLAYER_STOPPED_SHORT() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
    static int EV_TYPE_PATH_NOT_FOUND() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
    static int EV_TYPE_LICENSE_LOCK_DID_TRIGGER() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
    
    static PlayerController* createPlayerController(PuzzleBoard *board = NULL);
	virtual ~PlayerController(void);
    
    virtual void reset(void) { }
	virtual void updateBoardBounds(void) { }
    virtual void advanceTime(float dt) { }
    virtual void onEvent(int evType, void* evData) { }
    
    bool isEnabled(void) const { return mEnabled; }
    void enable(bool value) { mEnabled = value; }
    bool isLicenseLocked(void) const { return mLicenseLocked; }
    void setLicenseLocked(bool value) { mLicenseLocked = value; }
    Player* getPlayer(void) { return mPlayer; }
	virtual void setPlayer(Player* value);
    virtual const Coord* getPath(void) const { return 0; }
	virtual int getPathLength(void) const { return 0; }
    virtual void setPathFinder(PathFinder* value) { }
    
    virtual void playerValueDidChange(uint code, int value) { }
	virtual void willBeginMoving(void) { }
    virtual void didFinishMoving(void) { }
    virtual void didIdle(void) { }
    
    virtual void puzzleSoundShouldPlay(const char* soundName) { }
	virtual void puzzlePlayerWillMove(Player* player) { }
	virtual void puzzlePlayerDidMove(Player* player) { }
	virtual void puzzleShieldDidDeploy(int tileIndex) { }
	virtual void puzzleShieldWasWithdrawn(int tileIndex) { }
    virtual void puzzleTilesShouldRotate(const int2d& tileIndexes) { }
    virtual void puzzleTileSwapWillBegin(const int2d& swapIndexes, bool isCenterValid) { }
	virtual void puzzleConveyorBeltWillMove(Coord moveDir, int wrapIndex, const int1d& tileIndexes, int numTiles) { }
	virtual void puzzleWasSolved(int tileIndex) { }
    
    virtual unsigned int getInputFocus(void) const { return 0; };
	virtual void didGainFocus(void) { }
    virtual void update(int controllerState) { }
    
protected:
    PlayerController(void);
    
    Player* mPlayer;
    
private:
    bool mEnabled;
    bool mLicenseLocked;
};
#endif // __PLAYER_CONTROLLER_H__
