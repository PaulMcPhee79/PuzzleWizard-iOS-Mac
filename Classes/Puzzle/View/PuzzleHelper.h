#ifndef __PUZZLE_HELPER_H__
#define __PUZZLE_HELPER_H__

#include "cocos2d.h"
class PuzzleTile;
class TileDecoration;
USING_NS_CC;

namespace Puzzles
{
	const int kRedIndex = 0;
	const int kBlueIndex = 1;
	const int kGreenIndex = 2;
	const int kYellowIndex = 3;
	const int kWhiteIndex = 4;
	const int kNullIndex = 5;
    
    enum ColorScheme { CS_NORMAL, CS_COLOR_BLIND };
    
    ColorScheme getColorScheme(void);
    void setColorScheme(ColorScheme value);
    uint tileColorForIndex(int index);
    uint playerColorForIndex(int index);
    
	TileDecoration* decorationForTile(PuzzleTile* tile);
	uint colorForKey(uint key);
	uint playerColorForKey(uint key);
	uint pathColorForKey(uint key);
	void setColorSwapColorsForTile(uint decorationStyleKey, uint& colorLeft, uint& colorRight);
	uint tileFunctionForDecorationKey(uint decorationKey);
    uint colorForIQ(int IQ);
}

#endif // __PUZZLE_HELPER_H__
