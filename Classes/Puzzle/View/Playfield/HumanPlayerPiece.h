#ifndef __HUMAN_PLAYER_PIECE_H__
#define __HUMAN_PLAYER_PIECE_H__

#include "cocos2d.h"
#include <Puzzle/View/Playfield/AnimPlayerPiece.h>

USING_NS_CC;

class HumanPlayerPiece : public AnimPlayerPiece
{
public:
	HumanPlayerPiece(Player* player, bool shouldPlaySounds = true);
	virtual ~HumanPlayerPiece(void);

	static HumanPlayerPiece* create(Player* player, bool shouldPlaySounds = true, bool autorelease = true);

protected:
    virtual const char* getIdleMidFramesPrefix() { return "idle_rim_lge_"; };
    virtual const char* getMovingMidFramesPrefix() { return "walk_rim_lge_"; };
    virtual int getReuseKey(void) { return PlayerPiece::HUMAN; }
    
};
#endif // __HUMAN_PLAYER_PIECE_H__
