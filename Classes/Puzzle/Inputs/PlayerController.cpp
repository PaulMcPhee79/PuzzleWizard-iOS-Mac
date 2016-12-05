

#include "PlayerController.h"
#include "PlayerControllerIOS.h"
#include "PlayerControllerOSX.h"
#include <Puzzle/View/Playfield/PuzzleBoard.h>

PlayerController* PlayerController::createPlayerController(PuzzleBoard *board)
{
#ifdef CHEEKY_MOBILE
    return new PlayerControllerIOS(board);
#else
    return new PlayerControllerOSX();
#endif
}

PlayerController::PlayerController(void)
:
mEnabled(true),
mLicenseLocked(false),
mPlayer(NULL)
{
    
}

PlayerController::~PlayerController(void)
{
	CC_SAFE_RELEASE_NULL(mPlayer);
}

void PlayerController::setPlayer(Player* value)
{
	if (mPlayer == value)
		return;
    
	CC_SAFE_RETAIN(value);
	CC_SAFE_RELEASE(mPlayer);
    
    mPlayer = value;
}
