
#include "MirrorPlayerPiece.h"
#include <Utils/PWDebug.h>
USING_NS_CC;

MirrorPlayerPiece::MirrorPlayerPiece(Player* player, bool shouldPlaySounds)
	: AnimPlayerPiece(player, shouldPlaySounds)
{
    PWDebug::mirrorPlayerPieceCount++;
}

MirrorPlayerPiece::~MirrorPlayerPiece(void)
{
	
}

MirrorPlayerPiece* MirrorPlayerPiece::create(Player* player, bool shouldPlaySounds, bool autorelease)
{
    MirrorPlayerPiece *playerPiece = new MirrorPlayerPiece(player, shouldPlaySounds);
    if (playerPiece && playerPiece->init())
    {
		if (autorelease)
			playerPiece->autorelease();
        return playerPiece;
    }
    CC_SAFE_DELETE(playerPiece);
    return NULL;
}
