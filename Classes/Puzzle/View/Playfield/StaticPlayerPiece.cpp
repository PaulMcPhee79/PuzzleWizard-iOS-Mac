
#include "StaticPlayerPiece.h"
#include <Utils/CMTypes.h>
#include <Utils/Utils.h>
#include <Puzzle/View/PuzzleHelper.h>
USING_NS_CC;

StaticPlayerPiece::StaticPlayerPiece(Player* player)
	: PlayerPiece(player)
{

}

StaticPlayerPiece::~StaticPlayerPiece(void)
{
	CC_SAFE_RELEASE_NULL(mCostume);
}

StaticPlayerPiece* StaticPlayerPiece::create(Player* player, bool autorelease)
{
	StaticPlayerPiece *playerPiece = new StaticPlayerPiece(player);
    if (playerPiece && playerPiece->init())
    {
		if (autorelease)
			playerPiece->autorelease();
        return playerPiece;
    }
    CC_SAFE_DELETE(playerPiece);
    return NULL;
}

bool StaticPlayerPiece::init(void)
{
	bool bRet = PlayerPiece::init();
	if (bRet)
	{
		mCostume = new CCSprite();
		bRet = mCostume->initWithSpriteFrame(mScene->spriteFrameByName("idle_body_s_00"));
		if (bRet)
		{
			addChild(mCostume);
			syncWithData();
		}
	}

	return bRet;
}

void StaticPlayerPiece::syncWithData(void)
{
	PlayerPiece::syncWithData();
	Player* player = getPlayer();
	if (player && mCostume)
	{
		Coord pos = player->getPosition();
		vec2 moveDims = player->getMoveDimensions();
		setPosition(ccp(pos.x * moveDims.x, pos.y * moveDims.y));
		mCostume->setColor(CMUtils::uint2color3B(Puzzles::playerColorForKey(player->getColorKey())));
	}
}
