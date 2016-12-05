#ifndef __MIRROR_PLAYER_PIECE_H__
#define __MIRROR_PLAYER_PIECE_H__

#include "cocos2d.h"
#include <Puzzle/View/Playfield/AnimPlayerPiece.h>

USING_NS_CC;

class MirrorPlayerPiece : public AnimPlayerPiece
{
public:
	MirrorPlayerPiece(Player* player, bool shouldPlaySounds = true);
	virtual ~MirrorPlayerPiece(void);

	static MirrorPlayerPiece* create(Player* player, bool shouldPlaySounds = true, bool autorelease = true);
    
protected:
    virtual const char* getIdleMidFramesPrefix() { return "idle_rim_sml_"; };
    virtual const char* getMovingMidFramesPrefix() { return "walk_rim_sml_"; };
    virtual int getReuseKey(void) { return PlayerPiece::MIRROR; }
    
};
#endif // __MIRROR_PLAYER_PIECE_H__
