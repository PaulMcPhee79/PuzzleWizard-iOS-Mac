#ifndef __CM_FILE_TYPES_H__
#define __CM_FILE_TYPES_H__

#include "cocos2d.h"

const int ksXLevelNameLen = 32;
const int ksXPuzzleNameLen = 32;

typedef struct sXLevel
{
	char name[ksXLevelNameLen];
	int numPuzzles;
} sXLevel;

typedef struct sXPuzzle
{
	char name[ksXPuzzleNameLen];
    int iq;
	int numColumns;
	int numRows;
	int numPlayers;
} sXPuzzle;

typedef struct sXTile
{
	uint devProperties;
	uint devPainter;
} sXTile;

typedef struct sXPlayer
{
	uint devColorKey;
	uint devOrientation;
	int devPosX;
	int devPosY;
} sXPlayer;

#endif // __CM_FILE_TYPES_H__
