
#define TILE_BRIGHTNESS 50
#define PLAYER_BRIGHTNESS 75

#include "PuzzleHelper.h"
#include <Puzzle/Data/PuzzleTile.h>
#include <Puzzle/View/Playfield/TilePiece.h>
#include <Puzzle/View/Playfield/TileDecoration.h>
#include <Utils/Globals.h>
USING_NS_CC;

namespace Puzzles
{
    static const int kNumColors = kNullIndex + 1;
#if TILE_BRIGHTNESS == 100
    // 100% brightness
    static const uint kNormalTileColors[kNumColors] =
    {
        0xd61906, 0x1d9fe8, 0x00e125, 0xfffd2f, 0xf8f8f8, 0x808080
    };
#elif TILE_BRIGHTNESS == 75
    // 75% brightness
    static const uint kNormalTileColors[kNumColors] =
    {
        0xd62d1c, 0x33a7e8, 0x16e138, 0xfffd40, 0xf8f8f8, 0x808080
    };
#elif TILE_BRIGHTNESS == 50
    // 50% brightness
    static const uint kNormalTileColors[kNumColors] =
    {
        0xd64031, 0x4aafe8, 0x2de14b, 0xfffd59, 0xf2f2f2, 0x808080
    };
#endif
    
#if PLAYER_BRIGHTNESS == 100
    static const uint kNormalPlayerColors[kNumColors] =
    {
        0x960000, 0x005ba2, 0x008516, 0xc1a300, 0xd1d1d1, 0x808080
    };
#elif PLAYER_BRIGHTNESS == 75
    static const uint kNormalPlayerColors[kNumColors] =
    {
        0xa60000, 0x006bbd, 0x009418, 0xd2b200, 0xd1d1d1, 0x808080
    };
#endif
    
    static const uint kColorBlindTileColors[kNumColors] =
    {
        0xff0000, 0x0066cc, 0x00ccff, 0xffcc00, 0xf8f8f8, 0x808080
    };
    
    static const uint kColorBlindPlayerColors[kNumColors] =
    {
        0xcc0000, 0x003399, 0x0099ff, 0xff9900, 0xcbcbcb, 0x808080
    };
    
    static const uint kIQColors[3] =
    {
        0x00ff00, 0xff8800, 0xff0000
    };
    
    static ColorScheme s_ColorScheme = Puzzles::CS_NORMAL;
    static const uint* s_TileColors = kNormalTileColors;
    static const uint* s_PlayerColors = kNormalPlayerColors;
    
    ColorScheme getColorScheme(void)
    {
        return s_ColorScheme;
    }
    
    void setColorScheme(ColorScheme value)
    {
        if (value == s_ColorScheme)
            return;
        
        switch (value)
        {
            case Puzzles::CS_COLOR_BLIND:
                s_TileColors = kColorBlindTileColors;
                s_PlayerColors = kColorBlindPlayerColors;
                break;
            case Puzzles::CS_NORMAL:
            default:
                s_TileColors = kNormalTileColors;
                s_PlayerColors = kNormalPlayerColors;
                break;
        }
        
        s_ColorScheme = value;
    }
    
    uint tileColorForIndex(int index)
    {
        CCAssert(index >= 0 && index < kNumColors, "PuzzleHelper::tileColorForIndex = index out of range.");
        return s_TileColors[index];
    }
    
    uint playerColorForIndex(int index)
    {
        CCAssert(index >= 0 && index < kNumColors, "PuzzleHelper::playerColorForIndex = index out of range.");
        return s_PlayerColors[index];
    }
    
	TileDecoration* decorationForTile(PuzzleTile* tile)
	{
		uint type = tile->getDecorationKey(), subType = 0;

        switch (type)
        {
			case TilePiece::kTDKTeleport:
                subType = tile->getDecorationStyleKey() >> PuzzleTile::kBitShiftDecorationStyle;
                break;
            case TilePiece::kTDKColorSwap:
                subType = tile->getDecorationStyleKey() >> PuzzleTile::kBitShiftDecorationStyle;
                break;
            case TilePiece::kTDKRotate:
                break;
            case TilePiece::kTDKShield:
                break;
            case TilePiece::kTDKPainter:
                subType = tile->getPainter();
                break;
            case TilePiece::kTDKTileSwap:
                subType = tile->getDecorationStyleKey() >> PuzzleTile::kBitShiftDecorationStyle;
                break;
            case TilePiece::kTDKMirroredImage:
                break;
            case TilePiece::kTDKKey:
                break;
			case TilePiece::kTDKColorFill:
                subType = tile->getPainter();
                break;
            case TilePiece::kTDKPaintBomb:
                break;
            case TilePiece::kTDKConveyorBelt:
                subType = tile->getDecorationStyleKey() >> PuzzleTile::kBitShiftDecorationStyle;
                break;
            case TilePiece::kTDKColorMagic:
                break;
            default:
                return NULL;
        }

		return TileDecoration::getTileDecoration(type, subType);
	}

	uint colorForKey(uint key)
	{
		uint color;

        switch (key)
        {
            case 1: color = tileColorForIndex(kRedIndex); break;
            case 2: color = tileColorForIndex(kBlueIndex); break;
            case 3: color = tileColorForIndex(kGreenIndex); break;
            case 4: color = tileColorForIndex(kYellowIndex); break;
            case 5: color = tileColorForIndex(kWhiteIndex); break;
            default: color = tileColorForIndex(kNullIndex); break;
        }

        return color;
	}

	uint playerColorForKey(uint key)
	{
		uint color;

        switch (key)
        {
            case 1: color = playerColorForIndex(kRedIndex); break;
            case 2: color = playerColorForIndex(kBlueIndex); break;
            case 3: color = playerColorForIndex(kGreenIndex); break;
            case 4: color = playerColorForIndex(kYellowIndex); break;
            case 5: color = playerColorForIndex(kWhiteIndex); break;
            default: color = playerColorForIndex(kNullIndex); break;
        }

        return color;
	}

	void setColorSwapColorsForTile(uint decorationStyleKey, uint& colorLeft, uint& colorRight)
	{
		// rb,rg,ry,rw,bg,by,bw,gy,gw,yw
        switch (decorationStyleKey)
        {
            case 0: colorLeft = tileColorForIndex(kRedIndex); colorRight = tileColorForIndex(kBlueIndex); break;
            case 1: colorLeft = tileColorForIndex(kRedIndex); colorRight = tileColorForIndex(kGreenIndex); break;
            case 2: colorLeft = tileColorForIndex(kRedIndex); colorRight = tileColorForIndex(kYellowIndex); break;
            case 3: colorLeft = tileColorForIndex(kRedIndex); colorRight = tileColorForIndex(kWhiteIndex); break;
            case 4: colorLeft = tileColorForIndex(kBlueIndex); colorRight = tileColorForIndex(kGreenIndex); break;
            case 5: colorLeft = tileColorForIndex(kBlueIndex); colorRight = tileColorForIndex(kYellowIndex); break;
            case 6: colorLeft = tileColorForIndex(kBlueIndex); colorRight = tileColorForIndex(kWhiteIndex); break;
            case 7: colorLeft = tileColorForIndex(kGreenIndex); colorRight = tileColorForIndex(kYellowIndex); break;
            case 8: colorLeft = tileColorForIndex(kGreenIndex); colorRight = tileColorForIndex(kWhiteIndex); break;
            case 9: colorLeft = tileColorForIndex(kYellowIndex); colorRight = tileColorForIndex(kWhiteIndex); break;
            default: colorLeft = 0; colorRight = 0; break;
        }
	}

	uint tileFunctionForDecorationKey(uint decorationKey)
	{
		uint tileFunction = PuzzleTile::kTFNone;

        switch (decorationKey)
        {
			case TilePiece::kTDKTeleport: tileFunction = PuzzleTile::kTFTeleport; break;
            case TilePiece::kTDKColorSwap: tileFunction = PuzzleTile::kTFColorSwap; break;
            case TilePiece::kTDKRotate: tileFunction = PuzzleTile::kTFRotate; break;
            case TilePiece::kTDKShield: tileFunction = PuzzleTile::kTFShield; break;
            case TilePiece::kTDKPainter: tileFunction = PuzzleTile::kTFPainter; break;
            case TilePiece::kTDKTileSwap: tileFunction = PuzzleTile::kTFTileSwap; break;
            case TilePiece::kTDKMirroredImage: tileFunction = PuzzleTile::kTFMirroredImage; break;
            case TilePiece::kTDKKey: tileFunction = PuzzleTile::kTFKey; break;
            case TilePiece::kTDKColorFill: tileFunction = PuzzleTile::kTFColorFill; break;
            case TilePiece::kTDKPaintBomb: tileFunction = PuzzleTile::kTFPaintBomb; break;
            case TilePiece::kTDKConveyorBelt: tileFunction = PuzzleTile::kTFConveyorBelt; break;
            case TilePiece::kTDKColorMagic: tileFunction = PuzzleTile::kTFColorMagic; break;
            default: tileFunction = PuzzleTile::kTFNone; break;
        }

        return tileFunction;
	}
    
    uint colorForIQ(int IQ)
    {
        if (IQ <= 99)
            return kIQColors[0];
        else if (IQ <= 125)
            return kIQColors[1];
        else
            return kIQColors[2];
    }
}
