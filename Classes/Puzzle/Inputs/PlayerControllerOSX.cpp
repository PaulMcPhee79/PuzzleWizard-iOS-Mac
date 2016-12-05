
#include "PlayerControllerOSX.h"
#include <Managers/InputManager.h>

PlayerControllerOSX::PlayerControllerOSX(void)
:
mMovementVector(CMCoordZero)
{

}

PlayerControllerOSX::~PlayerControllerOSX(void)
{

}

void PlayerControllerOSX::reset(void)
{
    PlayerController::reset();
    mMovementVector = CMCoordZero;
}

void PlayerControllerOSX::willLoseFocus(void)
{
    reset();
}

void PlayerControllerOSX::update(int controllerState)
{
    mMovementVector = InputManager::IM()->getHeldVector();
}

void PlayerControllerOSX::advanceTime(float dt)
{
    if (isEnabled() && mPlayer)
        mPlayer->setQueuedMove(mMovementVector);
}


