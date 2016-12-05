#ifndef __STATIC_PLAYER_PIECE_H__
#define __STATIC_PLAYER_PIECE_H__

#include "cocos2d.h"
#include <Puzzle/View/Playfield/PlayerPiece.h>
USING_NS_CC;

class StaticPlayerPiece : public PlayerPiece
{
public:
	StaticPlayerPiece(Player* player);
	virtual ~StaticPlayerPiece(void);

	static StaticPlayerPiece* create(Player* player, bool autorelease = true);
	virtual bool init(void);

	virtual void syncWithData(void);

private:
    virtual int getReuseKey(void) { return PlayerPiece::STATIC; }
    
	CCSprite* mCostume;
};
#endif // __STATIC_PLAYER_PIECE_H__
