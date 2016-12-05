
#include "HumanPlayerPiece.h"
#include <Utils/PWDebug.h>
USING_NS_CC;

HumanPlayerPiece::HumanPlayerPiece(Player* player, bool shouldPlaySounds)
	: AnimPlayerPiece(player, shouldPlaySounds)
{
    PWDebug::humanPlayerPieceCount++;
}

HumanPlayerPiece::~HumanPlayerPiece(void)
{
	
}

HumanPlayerPiece* HumanPlayerPiece::create(Player* player, bool shouldPlaySounds, bool autorelease)
{
    HumanPlayerPiece *playerPiece = new HumanPlayerPiece(player, shouldPlaySounds);
    if (playerPiece && playerPiece->init())
    {
		if (autorelease)
			playerPiece->autorelease();
        return playerPiece;
    }
    CC_SAFE_DELETE(playerPiece);
    return NULL;
}
