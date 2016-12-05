#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include "cocos2d.h"
#include "ccMacros.h"
USING_NS_CC;

namespace CMGlobals
{
	enum PFCat
	{
        SUB_BG = 0,
		BG,
        PLAYER_HUD,
        BOARD,
        PLAYER,
        SUB_DIALOG,
        DIALOGS,
        HUD,
        SPLASH,
		PFCAT_COUNT
	};
}

#endif // __GLOBALS_H__
