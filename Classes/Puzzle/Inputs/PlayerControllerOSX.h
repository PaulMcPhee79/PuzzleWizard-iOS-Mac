
#ifndef __PLAYER_CONTROLLER_OSX_H__
#define __PLAYER_CONTROLLER_OSX_H__

#include <Puzzle/Inputs/PlayerController.h>
#include <Puzzle/Data/Player.h>
#include <Managers/InputTypes.h>
#include <Utils/CMTypes.h>

class PlayerControllerOSX : public PlayerController
{
public:
	PlayerControllerOSX(void);
	virtual ~PlayerControllerOSX(void);
    
    virtual unsigned int getInputFocus(void) const { return CMInputs::HAS_FOCUS_BOARD; };
	virtual void willLoseFocus(void);
    virtual void update(int controllerState);
    virtual void advanceTime(float dt);
    
    virtual void reset(void);
    Coord getMovementVector(void) const { return mMovementVector; }
    
private:
    Coord mMovementVector;
};
#endif // __PLAYER_CONTROLLER_OSX_H__
