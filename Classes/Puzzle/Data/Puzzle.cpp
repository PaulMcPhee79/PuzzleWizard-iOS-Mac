
#include "Puzzle.h"
#include <Puzzle/View/Playfield/TilePiece.h>
#include <Puzzle/Data/ColorFiller.h>
#include <Puzzle/Data/HumanPlayer.h>
#include <Puzzle/Data/MirroredPlayer.h>
#include <Puzzle/Controllers/GameProgressController.h>
#include <Utils/CloudWrapper.h>
#include <Utils/CMFileTypes.h>
#include <Utils/DeviceDetails.h>
#include <Managers/FileManager.h>
#include <Utils/PWDebug.h>
#include <cmath>
USING_NS_CC;

typedef std::set<IPuzzleView*> PuzzleViews;

static const int kPaintBombIndexOffsets_8x6[] = { 9, 1, 0, 8, 16, 17, 18, 10, 2 };
static const int kPaintBombIndexOffsets_10x8[] = { 11, 1, 0, 10, 20, 21, 22, 12, 2 };

const int* Puzzle::kPaintBombIndexOffsets = NULL;
unsigned char* Puzzle::serializeBuffer = NULL;

#pragma warning( disable : 4351 ) // new behavior: elements of array 'Puzzle::mQueuedCommands' will be default initialized (changed in Visual C++ 2005) 
Puzzle::Puzzle(int id, const char* name, int numColumns, int numRows)
	: mID(id), mIQ(100), mName(name), mNumColumns(MAX(1, numColumns)), mNumRows(MAX(1, numRows)),
    mQueuedCommands(), mRotationIndexes(2, 9), mTileSwapIndexes(2, 9), mColorFiller(NULL),
	mVertConveyorBeltIndexes(MAX(1, numRows)), mHorizConveyorBeltIndexes(MAX(1, numColumns)),
	mLevelIndex(-1), mPuzzleIndex(-1), mConveyorBeltWrapIndex(-1),
	mSolved(false), mIsRotating(false), mIsTileSwapping(false), mIsConveyorBeltActive(false),
    mIsSearchWeightingEnabled(true), mResetting(false), mPaused(false), mViewsLocked(false),
    mShouldSerialize(false), mWasSerializationSuccessful(false), mSerializeState(Puzzle::SERIALIZE_NONE),
	mCommandPriorities(Puzzle::createCommandPriorities()), mSearchWeightings(Puzzle::createSearchWeightings())
{
    PWDebug::puzzleCount++;
    
    if (kPaintBombIndexOffsets == NULL)
    {
        kPaintBombIndexOffsets = MODE_8x6 ? kPaintBombIndexOffsets_8x6 : kPaintBombIndexOffsets_10x8;
    }
    
	mTiles = new PuzzleTile[mNumColumns * mNumRows];
    for (int i = 0, n = mNumColumns * mNumRows; i < n; ++i)
        mTiles[i].setEdgeTile(i >= mNumColumns * (mNumRows-1));
	mPlayers = new CCArray(); 
	mViews = new PuzzleViews();
	mConveyorBeltDir = CMCoordZero;

	//for (int i = 0; i < mNumColumns * mNumRows; ++i)
	//	mTiles[i].setColorKey(TilePiece::kColorKeyBlue);
}

Puzzle::~Puzzle(void)
{
	CC_SAFE_RELEASE_NULL(mPlayers);
	CC_SAFE_RELEASE_NULL(mColorFiller);

    // * POTENTIAL BUG *
	// Late in development it was realized that this bypasses CCObject's retain/release cycle and
    // could thus result in stray pointers being dereferenced by client classes. However, it appears
    // that we outlive all clients and so this has never been an issue. It has been left for now
    // because there has never been a crash with PuzzleWizard, but do consider this possibility.
	if (mTiles)
	{
		delete[] mTiles;
		mTiles = NULL;
	}

	delete mViews, mViews = NULL;
}

Puzzle* Puzzle::create(int id, const char* name, int numColumns, int numRows, bool autorelease)
{
	Puzzle *puzzle = new Puzzle(id, name, numColumns, numRows);
    if (puzzle)
    {
		if (autorelease)
			puzzle->autorelease();
        return puzzle;
    }
    CC_SAFE_DELETE(puzzle);
    return NULL;
}

Puzzle* Puzzle::createFromStream(int id, u8* stream, ulong* offset, bool autorelease)
{
	sXPuzzle* sxpuzzle = (sXPuzzle*)(stream + *offset);
	Puzzle* puzzle = Puzzle::create(id, sxpuzzle->name, sxpuzzle->numColumns, sxpuzzle->numRows, autorelease);
	
	if (puzzle)
	{
        puzzle->setIQ(sxpuzzle->iq);
        
		*offset += sizeof(*sxpuzzle);
		int numTiles = sxpuzzle->numColumns * sxpuzzle->numRows;
		for (int i = 0; i < numTiles; ++i)
		{
			sXTile* sxtile = (sXTile*)(stream + *offset);
			PuzzleTile* tile = puzzle->tileAtIndex(i);
			tile->setDevProperties(sxtile->devProperties);
			tile->setDevPainter(sxtile->devPainter);
			*offset += sizeof(*sxtile);
		}

		for (int i = 0; i < sxpuzzle->numPlayers; ++i)
		{
			sXPlayer* sxplayer = (sXPlayer*)(stream + *offset);
			HumanPlayer* player = HumanPlayer::create(sxplayer->devColorKey, cmc(sxplayer->devPosX, sxplayer->devPosY), sxplayer->devOrientation);
			if (player)
				puzzle->addPlayer(player);
			*offset += sizeof(*sxplayer);
		}
	}
	
	return puzzle;
}

//const uint k2TFNone = 0;
const uint k2TFTeleport = 1 << 16;
const uint k2TFColorSwap = 2 << 16;
const uint k2TFRotate = 3 << 16;
const uint k2TFShield = 4 << 16;
const uint k2TFPainter = 5 << 16;
const uint k2TFTileSwap = 6 << 16;
const uint k2TFMirroredImage = 7 << 16;
const uint k2TFKey = 8 << 16;
const uint k2TFColorFill = 9 << 16;
const uint k2TFPaintBomb = 10 << 16;
const uint k2TFConveyorBelt = 11 << 16;
const uint k2TFColorMagic = 12 << 16;

std::map<uint, int> Puzzle::createCommandPriorities(void)
{
	std::map<uint, int> m;
	m[k2TFKey] = 1;
	m[k2TFTileSwap] = 2;
	m[k2TFMirroredImage] = 3;
	m[k2TFShield] = 4;
	m[k2TFTeleport] = 5;
	m[k2TFColorSwap] = 6;
	m[k2TFPaintBomb] = 7;
	m[k2TFPainter] = 8;
	m[k2TFRotate] = 9;
	m[k2TFConveyorBelt] = 10;
	m[k2TFColorFill] = 11;
	m[k2TFColorMagic] = 12;
	return m;
}

std::map<uint, int> Puzzle::createSearchWeightings(void)
{
	std::map<uint, int> m;
	m[k2TFKey] = 0;
	m[k2TFTileSwap] = 3;
	m[k2TFMirroredImage] = 4;
	m[k2TFShield] = 2;
	m[k2TFTeleport] = 8;
	m[k2TFColorSwap] = 3;
	m[k2TFPaintBomb] = 6;
	m[k2TFPainter] = 7;
	m[k2TFRotate] = 4;
	m[k2TFConveyorBelt] = 4;
	m[k2TFColorFill] = 5;
	m[k2TFColorMagic] = 1;
	return m;
}

bool Puzzle::isModified(uint modifier, int originIndex, int numColumns, int numRows) const
{
	for (int i = 0; i < numColumns; ++i)
    {
        for (int j = 0; j < numRows; ++j)
        {
            int index = originIndex + i * getNumColumns() + j;
            if (isValidIndex(index))
            {
                PuzzleTile* tile = tileAtIndex(index);
				if (tile->isModified(modifier))
                    return true;
            }
        }
    }

    return false;
}

bool Puzzle::isIndexAxiallyAdjacent(int index, int other) const
{
	int indexColumn = columnForIndex(index), indexRow = rowForIndex(index);
    int otherColumn = columnForIndex(other), otherRow = rowForIndex(other);
	return otherColumn >= 0 && otherColumn < getNumColumns() && otherRow >= 0 && otherRow < getNumRows()
        && (
			((indexColumn - otherColumn) == 0 && abs(indexRow - otherRow) == 1)
		||  ((indexRow - otherRow) == 0 && abs(indexColumn - otherColumn) == 1));
}

bool Puzzle::isIndexSurroundedBy(int index, int other) const
{
	int indexColumn = columnForIndex(index), indexRow = rowForIndex(index);
    int otherColumn = columnForIndex(other), otherRow = rowForIndex(other);
    return otherColumn >= 0 && otherColumn < getNumColumns() && otherRow >= 0 && otherRow < getNumRows()
        && abs(indexColumn - otherColumn) <= 1 && abs(indexRow - otherRow) <= 1;
}

Player* Puzzle::playerAtIndex(int index) const
{
	if (mPlayers && index >= 0 && index < getNumTiles())
    {
		CCObject* obj;
		CCARRAY_FOREACH(mPlayers, obj)
		{
			Player* player = static_cast<Player*>(obj);
			if (player && pos2Index(player->getPosition()) == index)
				return player;
		}
    }

    return NULL;
}

HumanPlayer* Puzzle::getAnyHumanPlayer(void) const
{
	HumanPlayer* player = NULL;

	if (mPlayers)
    {
		CCObject* obj;
		CCARRAY_FOREACH(mPlayers, obj)
		{
			player = dynamic_cast<HumanPlayer*>(obj);
			if (player)
				break;
		}
    }

	return player;
}

Player* Puzzle::getPlayerForID(int id) const
{
	Player* player = NULL;

	if (mPlayers)
    {
		CCObject* obj;
		CCARRAY_FOREACH(mPlayers, obj)
		{
			Player* p = static_cast<Player*>(obj);
			if (p && p->getID() == id)
			{
				player = p;
				break;
			}
		}
    }

	return player;
}

void Puzzle::addPlayer(Player* player)
{
	if (player && mPlayers && !mPlayers->containsObject(player))
    {
		if (player->getType() == Player::HUMAN_PLAYER)
        {
			mPlayers->addObject(player);
        }
        else
        {
            // We process moves in reverse order, but we must process human players prior to their mirrored counterparts.
			mPlayers->insertObject(player, 0);
        }

		dispatchEvent(EV_TYPE_PLAYER_ADDED(), player);
    }
}

void Puzzle::removePlayer(Player* player)
{
	if (player && mPlayers && mPlayers->containsObject(player))
    {
		player->retain();
		mPlayers->removeObject(player);
		dispatchEvent(EV_TYPE_PLAYER_REMOVED(), player);
		player->release();
    }
}

void Puzzle::removePlayerAtIndex(int index)
{
	if (mPlayers == NULL || index < 0 || index >= getNumTiles())
        return;

    Player* removeMe = NULL;
	CCObject* obj;
	CCARRAY_FOREACH(mPlayers, obj)
	{
		Player* player = static_cast<Player*>(obj);
		if (player && pos2Index(player->getPosition()) == index)
		{
			removeMe = player;
            break;
		}
	}

    if (removeMe)
        removePlayer(removeMe);
}

int Puzzle::rowForIndex(int index) const
{
	if (mTiles == NULL || !isValidIndex(index))
        return -1;
    else
        return index / getNumColumns();
}

int Puzzle::columnForIndex(int index) const
{
	if (mTiles == NULL || !isValidIndex(index))
        return -1;
    else
        return index % getNumColumns();
}

int Puzzle::indexForTile(const PuzzleTile* tile) const
{
	if (mTiles == NULL || tile == NULL)
        return -1;

	int numTiles = getNumTiles();
    for (int i = 0; i < numTiles; ++i)
    {
        if (&mTiles[i] == tile)
            return i;
    }

    return -1;
}

PuzzleTile* Puzzle::tileAtIndex(int index) const
{
	if (mTiles && isValidIndex(index))
        return &mTiles[index];
    else
        return NULL;
}

PuzzleTile* Puzzle::tileForTile(const PuzzleTile* tile) const
{
	if (mTiles == NULL || tile == NULL)
        return NULL;

    int numTiles = getNumTiles();
    for (int i = 0; i < numTiles; ++i)
    {
		PuzzleTile* t = &mTiles[i];
		if (t != tile && t->getFunctionKey() == tile->getFunctionKey() && t->getFunctionID() == tile->getFunctionID())
            return t;
    }

    return NULL;
}

CCArray* Puzzle::tileRange(int originIndex, int numColumns, int numRows)
{
	if (mTiles == NULL || originIndex >= getNumTiles() || numColumns <= 0 || numRows <= 0)
		return CCArray::create();

    originIndex = MAX(0, originIndex);
    int legalRangeX = MIN(numColumns, getNumColumns() - columnForIndex(originIndex)), legalRangeY = MIN(numRows, getNumRows() - rowForIndex(originIndex));
	CCArray* tiles = CCArray::createWithCapacity(legalRangeX * legalRangeY);

    for (int y = 0; y < legalRangeY; ++y)
    {
        int xOffset = originIndex + y * getNumColumns();
        for (int x = 0; x < legalRangeX; ++x)
			tiles->addObject(&mTiles[xOffset + x]);
    }

    return tiles;
}

void Puzzle::enableShield(int index, bool enable)
{
	PuzzleTile* centerTile = tileAtIndex(index);
    if (centerTile)
    {
        if (enable)
			centerTile->setModifiers(centerTile->getModifiers() | PuzzleTile::kTFShield);
        else
            centerTile->setModifiers(centerTile->getModifiers() & ~PuzzleTile::kTFShield);
    }

    int topLeftShieldIndex = index - (getNumColumns() + 1);
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            int shieldIndex = topLeftShieldIndex + i * getNumColumns() + j;
            if (isIndexSurroundedBy(index, shieldIndex)) // Prevent invalid tiles and wrap arounds
            {
                PuzzleTile* shieldTile = tileAtIndex(shieldIndex);

                if (enable)
					shieldTile->setModifiers(shieldTile->getModifiers() | PuzzleTile::kTMShielded);
                else
                    shieldTile->setModifiers(shieldTile->getModifiers() & ~PuzzleTile::kTMShielded);
            }
        }
    }
}

void Puzzle::refreshBoardFunction(uint function)
{
	switch (function)
    {
		case PuzzleTile::kTFShield:
        {
			int numTiles = getNumTiles();
			for (int i = 0; i < numTiles; ++i)
			{
				PuzzleTile* tile = &mTiles[i];
				if (tile->getFunctionKey() == PuzzleTile::kTFShield && tile->isModified(PuzzleTile::kTFShield))
                    enableShield(indexForTile(tile), true);
			}
        }
        break;
    }
}

void Puzzle::recolorPlayers(void)
{
	// Make sure all players are legally/validly positioned.
	CCObject* obj;
	CCARRAY_FOREACH(mPlayers, obj)
	{
		Player* player = static_cast<Player*>(obj);
		if (player)
		{
			int pIndex = pos2Index(player->getPosition());
			PuzzleTile* t = &mTiles[pIndex];
			if (player->getColorKey() != t->getColorKey())
				player->setColorKey(t->getColorKey());
		}
	}
}

bool Puzzle::isPlayerOccupied(const Player* player)
{
	if (player == NULL || (!isRotating() && !isConveyorBeltActive()))
        return false;

    bool isOccupied = false;
    int tileIndex = pos2Index(player->getPosition());

    if (isRotating())
    {
        for (int i = 0; i < 9; ++i)
        {
			if (tileIndex == mRotationIndexes.val(0, i))
            {
                isOccupied = true;
                break;
            }
        }
    }

    if (isConveyorBeltActive())
    {
        int1d* beltIndexes = getConveyorBeltIndexes();
		int beltLen = getConveyorBeltIndexCount();
        for (int i = 0; i < beltLen; ++i)
        {
			if (tileIndex == beltIndexes->at(i))
            {
                isOccupied = true;
                break;
            }
        }
    }

    return isOccupied;
}

void Puzzle::applyRotation(void)
{
	if (!isRotating())
        return;
    
    PuzzleTile swap, cache;

    for (int i = 0; i < 8; ++i)
    {
		PuzzleTile* rotateToTile = tileAtIndex(mRotationIndexes.val(1, i + 1));
		swap.copyProperties(*rotateToTile);
		rotateToTile->copyProperties(i == 0 ? *tileAtIndex(mRotationIndexes.val(0, i + 1)) : cache);
        cache.copyProperties(swap);
    }

	for (int i = 0; i < (int)mPlayers->count(); ++i)
    {
		Player* player = static_cast<Player*>(mPlayers->objectAtIndex((uint)i));
        int playerIndex = pos2Index(player->getPosition());
        player->setQueuedMove(CMCoordZero);

        for (int j = 0; j < 8; ++j)
        {
            if (playerIndex == mRotationIndexes.val(0, j + 1))
            {
                player->setQueuedMove(index2Pos(mRotationIndexes.val(1, j + 1)));
                break;
            }
        }
    }

    for (int i = 0; i < (int)mPlayers->count(); ++i)
    {
		Player* player = static_cast<Player*>(mPlayers->objectAtIndex((uint)i));
        Coord queuedMove = player->getQueuedMove();
        if (queuedMove.x != 0 || queuedMove.y != 0)
        {
            player->silentMoveTo(queuedMove);
            player->setQueuedMove(CMCoordZero);
        }
    }

    setIsRotating(false);
}

void Puzzle::applyTileSwap(void)
{
	if (!isTileSwapping())
        return;

	setIsTileSwapping(false);
}

void Puzzle::applyConveyorBelt(void)
{
	if (!isConveyorBeltActive())
        return;

    Coord ori = mConveyorBeltDir;
    CCAssert(abs(ori.x + ori.y) == 1, "Invalid Puzzle.ConveyorBeltDir orientation.");
    CCAssert(isValidIndex(getConveyorBeltWrapIndex()), "Invalid Puzzle.ConveyorBeltWrapIndex.");

    PuzzleTile wrapTile;
    int1d* beltIndexes = getConveyorBeltIndexes();

    int dir = ori.x != 0 ? ori.x : ori.y;

    if (dir == 1)
    {
		wrapTile.copyProperties(*tileAtIndex(beltIndexes->at(getConveyorBeltWrapIndex())));
        for (int i = getConveyorBeltIndexCount()-1; i > 0; --i)
			tileAtIndex(beltIndexes->at(i))->copyProperties(*tileAtIndex(beltIndexes->at(i - 1)));
		tileAtIndex(beltIndexes->at(0))->copyProperties(wrapTile);
    }
    else
    {
        wrapTile.copyProperties(*tileAtIndex(beltIndexes->at(getConveyorBeltWrapIndex())));
		int beltLen = getConveyorBeltIndexCount();
        for (int i = 0; i < beltLen - 1; ++i)
            tileAtIndex(beltIndexes->at(i))->copyProperties(*tileAtIndex(beltIndexes->at(i + 1)));
        tileAtIndex(beltIndexes->at(beltLen - 1))->copyProperties(wrapTile);
    }

	for (int i = 0; i < (int)mPlayers->count(); ++i)
    {
		Player* player = static_cast<Player*>(mPlayers->objectAtIndex(i));
        int playerIndex = pos2Index(player->getPosition());
        player->setQueuedMove(cmc(-1, -1)); // Use -1, -1 because this queued move is used as a tile index; not as a delta.

		int beltLen = getConveyorBeltIndexCount();
        for (int j = 0; j < beltLen; ++j)
        {
			if (playerIndex == beltIndexes->at(j))
            {
                if (dir == 1)
					player->setQueuedMove(index2Pos(beltIndexes->at((j + 1) % beltLen)));
                else
					player->setQueuedMove(index2Pos(beltIndexes->at(j > 0 ? j - 1 : beltLen-1)));
                break;
            }
        }
    }

	for (int i = 0; i < (int)mPlayers->count(); ++i)
    {
		Player* player = static_cast<Player*>(mPlayers->objectAtIndex(i));
        Coord queuedMove = player->getQueuedMove();
        if (queuedMove.x != -1 && queuedMove.y != -1)
           player->silentMoveTo(queuedMove);

        player->setQueuedMove(CMCoordZero);
    }

    setIsConveyorBeltActive(false);
}

void Puzzle::tryMove(Player* player)
{
    const int kMoveTypeIdle = 0, kMoveTypeTreadmill = 1, kMoveTypeMove = 2;
    int moveType = kMoveTypeIdle;
    Coord queuedMove, currentPos, newPos;
    do
    {
        if (player == NULL || player->isMoving() || isRotating() || isTileSwapping() || isConveyorBeltActive())
            break;
        
        queuedMove = player->getQueuedMove();
        if (queuedMove.x == 0 && queuedMove.y == 0)
            break;
        
        moveType = kMoveTypeTreadmill;
        
        currentPos = player->getPosition();
        newPos = cmc(currentPos.x + queuedMove.x, currentPos.y + queuedMove.y);
        
        if (!isValidPos(newPos))
            break;
        
        int index = pos2Index(newPos);
        if (isValidIndex(index) && isIndexAxiallyAdjacent(pos2Index(currentPos), index))
        {
            PuzzleTile* tile = tileAtIndex(index);
            if (tile->getColorKey() == player->getColorKey() || tile->getColorKey() == TilePiece::kColorKeyWhite ||
                player->getColorKey() == TilePiece::kColorKeyWhite || tile->getColorKey() == TilePiece::kColorKeyMulti || player->isColorMagicActive())
            {
                move(player, newPos);
                moveType = kMoveTypeMove;
                break;
            }
        }
    } while (false);
    
    if (player)
    {
        if (moveType != kMoveTypeMove)
            player->idle();
        if (moveType == kMoveTypeTreadmill)
            treadmill(player, queuedMove);
        if (moveType == kMoveTypeMove)
            puzzlePlayerWillMove(player);
    }
}

void Puzzle::move(Player* player, const Coord& pos)
{
	if (player)
	{
		player->setFutureColorKey(tileAtIndex(pos2Index(pos))->getColorKey());
		player->beginMoveTo(pos);
	}
}

void Puzzle::treadmill(Player* player, const Coord& dir)
{
    if (player)
		player->treadmill(dir);
}

void Puzzle::enableSearchWeighting(bool enable)
{
    mIsSearchWeightingEnabled = enable;
}

int Puzzle::getSearchWeighting(int index)
{
	PuzzleTile* tile = tileAtIndex(index);
	if (tile == NULL || !mIsSearchWeightingEnabled) return 0;

	uint key = tile->getFunctionKey();
	if (key == 0) return 0;

	int weighting = mSearchWeightings[key]; // Insert 0 weighting for unknown keys.
	switch (key)
	{
		case PuzzleTile::kTFColorSwap:
			{
				bool didColorSwap = false;
				uint colorKeyLeft, colorKeyRight;
				TilePiece::setColorKeysForSwapKey(tile->getDecorationStyleKey(), colorKeyLeft, colorKeyRight);
				int numTiles = getNumTiles();
				for (int i = 0; i < numTiles; ++i)
				{
					PuzzleTile* t = &mTiles[i];
					if (t->isModified(PuzzleTile::kTMShielded))
						continue;
					if (t->getColorKey() == colorKeyLeft || t->getColorKey() == colorKeyRight)
						didColorSwap = true;
				}

				if (!didColorSwap)
					weighting = 0;
			}
			break;
		case PuzzleTile::kTFPainter:
			{
			    uint painter = tile->getPainter();
			    bool didPaint = false;

			    for (int i = 0; i < 4 && !didPaint; ++i) // 4 directions
			    {
			        uint colorKey = (painter >> (i * 4)) & PuzzleTile::kColorKeyMask;
			        if (colorKey == TilePiece::kColorKeyNone)
			            continue;

			        int paintIndex = index;
			        bool endWhile = !isValidIndex(paintIndex);

			        while (!endWhile && !didPaint)
			        {
			            switch (i)
			            {
			                case 0:
			                    paintIndex = paintIndex - getNumColumns();
			                    break;
			                case 1:
			                    paintIndex = paintIndex + 1;
			                    endWhile = paintIndex % getNumColumns() == 0;
			                    break;
			                case 2:
			                    paintIndex = paintIndex + getNumColumns();
			                    break;
			                case 3:
			                    paintIndex = paintIndex - 1;
			                    endWhile = paintIndex % getNumColumns() == getNumColumns() - 1;
			                    break;
			            }

			            if (!endWhile)
			                endWhile = !isValidIndex(paintIndex);

			            if (!endWhile)
			            {
			                PuzzleTile* paintTile = tileAtIndex(paintIndex);
			                if (paintTile && !paintTile->isModified(PuzzleTile::kTMShielded) && paintTile->getColorKey() != colorKey)
			                    didPaint = true;
			            }
			        }
			    }

			    if (!didPaint)
			        weighting = 0;
			}
			break;
		case PuzzleTile::kTFPaintBomb:
			{
			    bool didPaint = false;
			    int topLeftIndex = index - (getNumColumns() + 1);
			    for (int i = 0; i < 3 && !didPaint; ++i)
			    {
			        for (int j = 0; j < 3 && !didPaint; ++j)
			        {
			            int paintIndex = topLeftIndex + kPaintBombIndexOffsets[3 * i + j];
			            if (isIndexSurroundedBy(index, paintIndex))
			            {
			                PuzzleTile* paintTile = &mTiles[paintIndex];
			                if (paintTile->getColorKey() != tile->getColorKey() && !paintTile->isModified(PuzzleTile::kTMShielded))
			                    didPaint = true;
			            }
			        }
			    }

			    if (!didPaint)
			        weighting = 0;
			}
			break;
		case PuzzleTile::kTFColorFill:
			{
			    if (tile->isModified(PuzzleTile::kTMShielded))
			        break;

				bool didFlood = false;
			    uint prevColorKey = tile->getColorKey(), nextColorKey = tile->getPainter() & PuzzleTile::kColorKeyMask;
			    if (prevColorKey != nextColorKey && nextColorKey != 0)
			    {
					for (int i = 0; i < 4 && !didFlood; ++i)
					{
						switch (i)
						{
							case 0: // North
								{
									PuzzleTile* adjTile = tileAtIndex(index - getNumColumns());
									didFlood = adjTile && adjTile->getColorKey() == prevColorKey;
								}
								break;
							case 1: // East
								{
									PuzzleTile* adjTile = tileAtIndex(index + 1);
									didFlood = adjTile && adjTile->getColorKey() == prevColorKey;
								}
								break;
							case 2: // South
								{
									PuzzleTile* adjTile = tileAtIndex(index + getNumColumns());
									didFlood = adjTile && adjTile->getColorKey() == prevColorKey;
								}
								break;
							case 3: // West
								{
									PuzzleTile* adjTile = tileAtIndex(index - 1);
									didFlood = adjTile && adjTile->getColorKey() == prevColorKey;
								}
								break;
						}
					}
			    }

				if (!didFlood)
			        weighting = 0;
			}
			break;
		case PuzzleTile::kTFRotate:
			{
			    bool locked = false;
			    do
			    {
					// Don't overwrite shared rotation indexes.
					if (isRotating())
                        break;

			        // Prevent a rotation tile from rotating tiles off the board
			        if (isPerimeterIndex(index))
			        {
			            locked = true;
			            break;
			        }

			        int rotateFromIndex = index - (getNumColumns() + 1), rotateToIndex = 0;

					mRotationIndexes.at(0, 0) = index;
					mRotationIndexes.at(1, 0) = index;

			        // Cannot rotate if any tiles are shielded
			        if (isModified(PuzzleTile::kTMShielded, rotateFromIndex, 3, 3))
			        {
			            locked = true;
			            break;
			        }

			        // ACW from top left
			        for (int i = 0; i < 8; ++i)
			        {
			            switch (i)
			            {
			                case 0: rotateToIndex = rotateFromIndex + getNumColumns(); break;
			                case 1: rotateToIndex = rotateFromIndex + getNumColumns(); break;
			                case 2: rotateToIndex = rotateFromIndex + 1; break;
			                case 3: rotateToIndex = rotateFromIndex + 1; break;
			                case 4: rotateToIndex = rotateFromIndex - getNumColumns(); break;
			                case 5: rotateToIndex = rotateFromIndex - getNumColumns(); break;
			                case 6: rotateToIndex = rotateFromIndex - 1; break;
			                case 7: rotateToIndex = rotateFromIndex - 1; break;
			            }

						mRotationIndexes.at(0, i + 1) = rotateFromIndex;
						mRotationIndexes.at(1, i + 1) = rotateToIndex;
			            rotateFromIndex = rotateToIndex;
			        }
			    }
			    while (false);

			    // Lock if we're moving the conveyor belt and any tiles overlap.
			    if (!locked && isConveyorBeltActive())
			    {
			        int1d* beltIndexes = getConveyorBeltIndexes();
					int beltLen = getConveyorBeltIndexCount();
			        for (int i = 0; i < beltLen; ++i)
			        {
			            for (int j = 0; j < 9; ++j)
			            {
							if (beltIndexes->at(i) == mRotationIndexes.val(0, j))
			                {
			                    locked = true;
			                    break;
			                }
			            }
			        }
			    }

			    if (locked)
					weighting = 0;
			}
			break;
		case PuzzleTile::kTFConveyorBelt:
        {
			bool locked = false;
			do
			{
				// Don't overwrite shared conveyor belt indexes.
				if (isConveyorBeltActive())
					break;

				mConveyorBeltDir = Player::orientation2Coord(tile->getDecorationStyleKey() >> PuzzleTile::kBitShiftDecorationStyle);
				int beltLen = getConveyorBeltIndexCount();

				if (mConveyorBeltDir.x != 0)
				{
					setConveyorBeltWrapIndex(mConveyorBeltDir.x == 1 ? beltLen - 1 : 0);

					int row = rowForIndex(index);
					for (int i = 0; i < beltLen; ++i)
						mHorizConveyorBeltIndexes[i] = row * getNumColumns() + i;
				}
				else
				{
					setConveyorBeltWrapIndex(mConveyorBeltDir.y == 1 ? beltLen - 1 : 0);

					int column = columnForIndex(index);
					for (int i = 0; i < beltLen; ++i)
						mVertConveyorBeltIndexes[i] = column + i * getNumColumns();
				}

				int1d* beltIndexes = getConveyorBeltIndexes();

				// Lock if any tile is shielded.
				for (int i = 0; i < beltLen; ++i)
				{
					PuzzleTile* beltTile = tileAtIndex(beltIndexes->at(i));
					if (beltTile && beltTile->isModified(PuzzleTile::kTMShielded))
					{
						locked = true;
						break;
					}
				}

				// Lock if we're rotating and any tiles overlap.
				if (!locked && isRotating())
				{
					for (int i = 0; i < beltLen; ++i)
					{
						for (int j = 0; j < 9; ++j)
						{
							if (beltIndexes->at(i) == mRotationIndexes.val(0, j))
							{
								locked = true;
								break;
							}
						}
					}
				}
			} while (false);

            if (locked)
				weighting = 0;
        }
        break;
	}

	return weighting;
}

int Puzzle::getCommandPriority(uint command)
{
	std::map<uint, int>::iterator it = mCommandPriorities.find(command);
	if (it != mCommandPriorities.end())
		return it->second;
    else
        return -1;
}

void Puzzle::addQueuedCommand(Player* player)
{
	if (player == NULL)
        return;

    for (int i = 0; i < kQueuedCommandsCapacity; ++i)
    {
        if (mQueuedCommands[i] == NULL)
        {
            mQueuedCommands[i] = player;
            break;
        }

        if (getCommandPriority(player->getFunction()) < getCommandPriority(mQueuedCommands[i]->getFunction()))
        {
            if (i == kQueuedCommandsCapacity - 1)
                mQueuedCommands[i] = player;
            else
            {
                Player* swap = mQueuedCommands[i];
                mQueuedCommands[i] = player;
                mQueuedCommands[i + 1] = swap;
            }

            break;
        }
    }
}

void Puzzle::playerMoved(Player* player)
{
    mShouldSerialize = !mSolved;
    
	Coord pos = player->getPosition();
    int index = pos2Index(pos);
    PuzzleTile* tile = tileAtIndex(index);

    player->setFunction(tile->getFunctionKey());

	if (player->getFunction() != PuzzleTile::kTFNone)
        addQueuedCommand(player);
	puzzlePlayerDidMove(player);
}

void Puzzle::processQueuedCommands(void)
{
	for (int qc = 0; qc < kQueuedCommandsCapacity; ++qc)
    {
		Player* player = mQueuedCommands[qc];
        if (player == NULL)
            break;

        Coord pos = player->getPosition();
        int index = pos2Index(pos);
        PuzzleTile* tile = tileAtIndex(index);

        if (tile->getColorKey() == TilePiece::kColorKeyMulti && !tile->isModified(PuzzleTile::kTMShielded))
            tile->setColorKey(player->getColorKey());
        if (tile->getColorKey() == TilePiece::kColorKeyWhite && player->getColorKey() != TilePiece::kColorKeyWhite)
            player->setColorKey(tile->getColorKey());
        if (player->getColorKey() == TilePiece::kColorKeyWhite && tile->getColorKey() != TilePiece::kColorKeyWhite && tile->getColorKey() != TilePiece::kColorKeyMulti)
            player->setColorKey(tile->getColorKey());

        if (tile->getFunctionKey() != player->getFunction())
            continue;

        switch (player->getFunction())
        {
			case PuzzleTile::kTFTeleport:
                {
                    PuzzleTile* teleportTile = tileForTile(tile);
                    if (teleportTile)
                    {
                        player->teleportTo(index2Pos(indexForTile(teleportTile)));
                        player->setColorKey(teleportTile->getColorKey());
						puzzlePlayerDidMove(player);
                    }
                }
                break;
			case PuzzleTile::kTFColorSwap:
                {
                    bool didColorSwap = false;
                    uint colorKeyLeft, colorKeyRight;
                    TilePiece::setColorKeysForSwapKey(tile->getDecorationStyleKey(), colorKeyLeft, colorKeyRight);
					int numTiles = getNumTiles();
                    for (int i = 0; i < numTiles; ++i)
                    {
						PuzzleTile* t = &mTiles[i];
						if (t->isModified(PuzzleTile::kTMShielded))
                            continue;
                        if (t->getColorKey() == colorKeyLeft)
                        {
                            t->setDecorator(tile->getFunctionKey());
                            t->setColorKey(colorKeyRight);
                            didColorSwap = true;
                        }
                        else if (t->getColorKey() == colorKeyRight)
                        {
                            t->setDecorator(tile->getFunctionKey());
                            t->setColorKey(colorKeyLeft);
                            didColorSwap = true;
                        }
                    }

                    //RecolorPlayers();
                    if (didColorSwap)
                        puzzleSoundShouldPlay("color-swap");
                }
                break;
			case PuzzleTile::kTFRotate:
                {
                    if (isRotating())
                        break;

                    bool locked = false;
                    do
                    {
                        // Prevent a rotation tile from rotating tiles off the board
                        if (isPerimeterIndex(index))
                        {
                            locked = true;
                            break;
                        }

                        PuzzleTile swap, cache;
                        int rotateFromIndex = index - (getNumColumns() + 1), rotateToIndex = 0;

						mRotationIndexes.at(0, 0) = index;
						mRotationIndexes.at(1, 0) = index;

                        // Cannot rotate if any tiles are shielded
                        if (isModified(PuzzleTile::kTMShielded, rotateFromIndex, 3, 3))
                        {
                            locked = true;
                            break;
                        }

                        // ACW from top left
                        for (int i = 0; i < 8; ++i)
                        {
                            switch (i)
                            {
                                case 0: rotateToIndex = rotateFromIndex + getNumColumns(); break;
                                case 1: rotateToIndex = rotateFromIndex + getNumColumns(); break;
                                case 2: rotateToIndex = rotateFromIndex + 1; break;
                                case 3: rotateToIndex = rotateFromIndex + 1; break;
                                case 4: rotateToIndex = rotateFromIndex - getNumColumns(); break;
                                case 5: rotateToIndex = rotateFromIndex - getNumColumns(); break;
                                case 6: rotateToIndex = rotateFromIndex - 1; break;
                                case 7: rotateToIndex = rotateFromIndex - 1; break;
                            }

							mRotationIndexes.at(0, i + 1) = rotateFromIndex;
							mRotationIndexes.at(1, i + 1) = rotateToIndex;
                            rotateFromIndex = rotateToIndex;
                        }
                    }
                    while (false);

                    // Lock if we're moving the conveyor belt and any tiles overlap.
                    if (!locked && isConveyorBeltActive())
                    {
                        int1d* beltIndexes = getConveyorBeltIndexes();
						int beltLen = getConveyorBeltIndexCount();
                        for (int i = 0; i < beltLen; ++i)
                        {
                            for (int j = 0; j < 9; ++j)
                            {
								if (beltIndexes->at(i) == mRotationIndexes.val(0, j))
                                {
                                    locked = true;
                                    break;
                                }
                            }
                        }
                    }

                    if (!locked)
                    {
                        setIsRotating(true);
                        puzzleTilesShouldRotate(getRotationIndexes());
                    }
                    else
                        puzzleSoundShouldPlay("locked");
                }
                break;
			case PuzzleTile::kTFShield:
				{
					bool enable = !tile->isModified(PuzzleTile::kTFShield);
					enableShield(index, enable);
					refreshBoardFunction(PuzzleTile::kTFShield);
					setShieldIndex(index);

					if (enable)
						puzzleShieldDidDeploy(getShieldIndex());
					else
						puzzleShieldWasWithdrawn(getShieldIndex());
				}
                break;
			case PuzzleTile::kTFPainter:
                {
                    uint painter = tile->getPainter();
                    int paintCount = 0;

                    for (int i = 0; i < 4; ++i) // 4 directions
                    {
                        uint colorKey = (painter >> (i * 4)) & PuzzleTile::kColorKeyMask;
                        if (colorKey == TilePiece::kColorKeyNone)
                            continue;

                        int paintIndex = index;
                        bool endWhile = !isValidIndex(paintIndex);

                        int decoratorIndex = 0;
                        while (!endWhile)
                        {
                            switch (i)
                            {
                                case 0:
                                    paintIndex = paintIndex - getNumColumns();
                                    break;
                                case 1:
                                    paintIndex = paintIndex + 1;
                                    endWhile = paintIndex % getNumColumns() == 0;
                                    break;
                                case 2:
                                    paintIndex = paintIndex + getNumColumns();
                                    break;
                                case 3:
                                    paintIndex = paintIndex - 1;
                                    endWhile = paintIndex % getNumColumns() == getNumColumns() - 1;
                                    break;
                            }

                            if (!endWhile)
                                endWhile = !isValidIndex(paintIndex);

                            if (!endWhile)
                            {
                                PuzzleTile* paintTile = tileAtIndex(paintIndex);
                                if (!paintTile->isModified(PuzzleTile::kTMShielded) && paintTile->getColorKey() != colorKey)
                                {
                                    paintTile->setDecorator(tile->getFunctionKey());
                                    paintTile->setDecoratorData(decoratorIndex);
                                    paintTile->setColorKey(colorKey);
                                    ++decoratorIndex;
                                    ++paintCount;
                                }
                            }
                        }
                    }

                    //RecolorPlayers();
                    if (paintCount > 0)
                        puzzleSoundShouldPlay(paintCount < 3 ? "color-arrow-short" : (paintCount < 6 ? "color-arrow-medium" : "color-arrow"));
                }
                break;
			case PuzzleTile::kTFTileSwap:
                {
                    if (isTileSwapping())
                    {
                        CCAssert(false, "Attempt to TileSwap while busy TileSwapping.");
                        break;
                    }

                    // Must cache original tile states in case their is an overlap.
                    bool isCenterValid = true;
					int numSwaps = 0;
                    PuzzleTile srcTiles[9], destTiles[9];
                    PuzzleTile* destTile = tileForTile(tile), swapTile;
                    if (destTile)
                    {
                        int destIndex = indexForTile(destTile);
                        int srcRow = rowForIndex(index), srcColumn = columnForIndex(index);
                        int destRow = rowForIndex(destIndex), destColumn = columnForIndex(destIndex);

                        resetTileSwapIndexes();

                        for (int i = 0; i < 3; ++i)
                        {
                            // Verify rows
                            int srcRowIter = srcRow + i - 1;
                            if (srcRowIter < 0 || srcRowIter >= getNumRows())
                                continue;

                            int desRowIter = destRow + i - 1;
                            if (desRowIter < 0 || desRowIter >= getNumRows())
                                continue;

                            for (int j = 0; j < 3; ++j)
                            {
                                // Verify columns
                                int srcColumnIter = srcColumn + j - 1;
                                if (srcColumnIter < 0 || srcColumnIter >= getNumColumns())
                                    continue;

                                int destColumnIter = destColumn + j - 1;
                                if (destColumnIter < 0 || destColumnIter >= getNumColumns())
                                    continue;

                                // Fetch verified tiles
                                PuzzleTile* srcTile = &mTiles[srcRowIter * getNumColumns() + srcColumnIter];
                                destTile = &mTiles[desRowIter * getNumColumns() + destColumnIter];

                                if (srcTile->isModified(PuzzleTile::kTMShielded) || destTile->isModified(PuzzleTile::kTMShielded))
                                {
                                    if (srcTile == tile)
                                    {
                                        // Set the center tile to be available for positioning in the view.
										mTileSwapIndexes.at(0, 3 * i + j) = indexForTile(srcTile);
                                        mTileSwapIndexes.at(1, 3 * i + j) = indexForTile(destTile);
                                        isCenterValid = false;
                                    }

                                    continue;
                                }

                                // Cache verified tiles
								srcTiles[3 * i + j].copyProperties(*srcTile);
								destTiles[3 * i + j].copyProperties(*destTile);

                                // Cache tile swap indexes for view clients
                                mTileSwapIndexes.at(0, 3 * i + j) = indexForTile(srcTile);
                                mTileSwapIndexes.at(1, 3 * i + j) = indexForTile(destTile);
								++numSwaps;
                            }
                        }

						if (numSwaps == 0)
							break;

                        puzzleTileSwapWillBegin(getTileSwapIndexes(), isCenterValid);
                        setIsTileSwapping(true);

                        // Shift cached tiles
                        for (int i = 0; i < 3; ++i)
                        {
                            int srcRowIter = srcRow + i - 1;
                            int desRowIter = destRow + i - 1;

                            for (int j = 0; j < 3; ++j)
                            {
                                int srcColumnIter = srcColumn + j - 1;
                                int destColumnIter = destColumn + j - 1;

								if (srcTiles[3 * i + j].getColorKey() != TilePiece::kColorKeyNone && destTiles[3 * i + j].getColorKey() != TilePiece::kColorKeyNone)
                                {
                                    PuzzleTile* srcTile = &mTiles[srcRowIter * getNumColumns() + srcColumnIter];
                                    destTile = &mTiles[desRowIter * getNumColumns() + destColumnIter];
									srcTile->copyProperties(destTiles[3 * i + j]);
                                    destTile->copyProperties(srcTiles[3 * i + j]);
                                }
                            }
                        }
                    }

                    //RecolorPlayers();
                }
                break;
			case PuzzleTile::kTFMirroredImage:
                {
                    PuzzleTile* mirroredTile = tileForTile(tile);
					HumanPlayer* humanPlayer = dynamic_cast<HumanPlayer*>(player);
                    if (mirroredTile && humanPlayer)
                    {
                        if (humanPlayer->getMirroredSelf())
                            removePlayer(humanPlayer->getMirroredSelf());
                        MirroredPlayer* mirroredPlayer = MirroredPlayer::create(mirroredTile->getColorKey(), index2Pos(indexForTile(mirroredTile)),
                                Player::getMirroredOrientation(player->getOrientation()));
                        humanPlayer->setMirroredSelf(mirroredPlayer);
                        addPlayer(mirroredPlayer);

                        puzzleSoundShouldPlay("mirrored-self");
                    }
                }
                break;
			case PuzzleTile::kTFKey:
                {
                    if (!mSolved)
                    {
                        if (!GameProgressController::GPC()->hasSolved(mLevelIndex, mPuzzleIndex))
                            dispatchEvent(EV_TYPE_UNSOLVED_PUZZLE_WILL_SOLVE(), this);
                        
                        puzzleSoundShouldPlay("solved");
                        mShouldSerialize = false;
                        mSolved = true;
						GameProgressController::GPC()->setSolved(mSolved, mLevelIndex, mPuzzleIndex);
						GameProgressController::GPC()->save();
                        
#ifdef __APPLE__
                        if (CloudWrapper::hasActiveCloudAccount())
                        {
                            ulong gpSize = 0;
                            const u8* gpData = GameProgressController::GPC()->exportData(gpSize);
                            if (gpData && gpSize > 0)
                                CloudWrapper::save(gpData, gpSize);
                        }
#elif defined(__ANDROID__)
                        CloudWrapper::save(0, 0);
#endif
                        puzzleWasSolved(indexForTile(tile));
                    }
                }
                break;
			case PuzzleTile::kTFColorFill:
                {
                    if (tile->isModified(PuzzleTile::kTMShielded))
                        break;

                    uint prevColorKey = tile->getColorKey(), nextColorKey = tile->getPainter() & PuzzleTile::kColorKeyMask;
                    if (prevColorKey != nextColorKey && nextColorKey != 0)
                    {
                        tile->setDecorator(PuzzleTile::kTFColorFill);
                        tile->setDecoratorData(0);
                        tile->setColorKey(nextColorKey);

						if (mColorFiller == NULL)
							mColorFiller = new ColorFiller();
                        
                        int paintCount = mColorFiller->fill(this, tile, prevColorKey);
                        if (paintCount > 0)
                            puzzleSoundShouldPlay(paintCount < 20 ? "color-flood-short" : (paintCount < 40 ? "color-flood-medium" : "color-flood"));
                    }
                }
                break;
			case PuzzleTile::kTFPaintBomb:
                {
                    int paintCount = 0;
                    int decoratorIndex = 0;

                    int topLeftIndex = index - (getNumColumns() + 1);
                    for (int i = 0; i < 3; ++i)
                    {
                        for (int j = 0; j < 3; ++j)
                        {
                            int paintIndex = topLeftIndex + kPaintBombIndexOffsets[3 * i + j];
                            if (isIndexSurroundedBy(index, paintIndex))
                            {
                                PuzzleTile* paintTile = &mTiles[paintIndex];
                                if (paintTile->getColorKey() != tile->getColorKey() && !paintTile->isModified(PuzzleTile::kTMShielded))
                                {
                                    paintTile->setDecorator(tile->getFunctionKey());
                                    paintTile->setDecoratorData(decoratorIndex);
                                    paintTile->setColorKey(tile->getColorKey());
                                    ++decoratorIndex;
                                    ++paintCount;
                                }
                            }
                        }
                    }

                    if (paintCount > 0)
                        puzzleSoundShouldPlay(paintCount < 3 ? "color-swirl-short" : (paintCount < 6 ? "color-swirl-medium" : "color-swirl"));
                }
                break;
			case PuzzleTile::kTFConveyorBelt:
                {
                    if (isConveyorBeltActive())
                        break;

                    mConveyorBeltDir = Player::orientation2Coord(tile->getDecorationStyleKey() >> PuzzleTile::kBitShiftDecorationStyle);
					int beltLen = getConveyorBeltIndexCount();

                    if (mConveyorBeltDir.x != 0)
                    {
                        setConveyorBeltWrapIndex(mConveyorBeltDir.x == 1 ? beltLen - 1 : 0);

                        int row = rowForIndex(index);
                        for (int i = 0; i < beltLen; ++i)
                            mHorizConveyorBeltIndexes[i] = row * getNumColumns() + i;
                    }
                    else
                    {
                        setConveyorBeltWrapIndex(mConveyorBeltDir.y == 1 ? beltLen - 1 : 0);

                        int column = columnForIndex(index);
                        for (int i = 0; i < beltLen; ++i)
                            mVertConveyorBeltIndexes[i] = column + i * getNumColumns();
                    }

                    bool locked = false;
                    int1d* beltIndexes = getConveyorBeltIndexes();

                    // Lock if any tile is shielded.
                    for (int i = 0; i < beltLen; ++i)
                    {
						PuzzleTile* beltTile = tileAtIndex(beltIndexes->at(i));
						if (beltTile && tileAtIndex(beltIndexes->at(i))->isModified(PuzzleTile::kTMShielded))
                        {
                            locked = true;
                            break;
                        }
                    }

                    // Lock if we're rotating and any tiles overlap.
                    if (!locked && isRotating())
                    {
                        for (int i = 0; i < beltLen; ++i)
                        {
                            for (int j = 0; j < 9; ++j)
                            {
								if (beltIndexes->at(i) == mRotationIndexes.val(0, j))
                                {
                                    locked = true;
                                    break;
                                }
                            }
                        }
                    }

                    if (!locked)
                    {
                        setIsConveyorBeltActive(true);
                        puzzleConveyorBeltWillMove(getConveyorBeltDir(), getConveyorBeltWrapIndex(), getConveyorBeltIndexes(), getConveyorBeltIndexCount());
                    }
                    else
                        puzzleSoundShouldPlay("locked");
                }
                break;
			case PuzzleTile::kTFColorMagic:
                player->setIsColorMagicActive(true);
                puzzleSoundShouldPlay("color-magic");
                break;
            default:
                break;
        }
    }

    for (int i = 0; i < kQueuedCommandsCapacity; ++i)
    {
        if (mQueuedCommands[i])
        {
            mQueuedCommands[i]->setFunction(0);
            mQueuedCommands[i] = NULL;
        }
        else
            break;
    }
}

void Puzzle::registerView(IPuzzleView* view)
{
	CCAssert(!mViewsLocked, "IPuzzleView is immutable during event propagation.");
	if (mViews == NULL)
		mViews = new PuzzleViews();
	if (view)
		mViews->insert(view);
}

void Puzzle::deregisterView(IPuzzleView* view)
{
	CCAssert(!mViewsLocked, "IPuzzleView is immutable during event propagation.");
	if (mViews && view)
		mViews->erase(view);
}

void Puzzle::puzzleSoundShouldPlay(const char* soundName)
{
	mViewsLocked = true;
	for(PuzzleViews::iterator it = mViews->begin(); it != mViews->end(); ++it)
		(*it)->puzzleSoundShouldPlay(soundName);
	mViewsLocked = false;
}

void Puzzle::puzzlePlayerWillMove(Player* player)
{
	mViewsLocked = true;
	for(PuzzleViews::iterator it = mViews->begin(); it != mViews->end(); ++it)
		(*it)->puzzlePlayerWillMove(player);
	mViewsLocked = false;
}

void Puzzle::puzzlePlayerDidMove(Player* player)
{
	mViewsLocked = true;
	for(PuzzleViews::iterator it = mViews->begin(); it != mViews->end(); ++it)
		(*it)->puzzlePlayerDidMove(player);
	mViewsLocked = false;
}

void Puzzle::puzzleShieldDidDeploy(int tileIndex)
{
	mViewsLocked = true;
	for(PuzzleViews::iterator it = mViews->begin(); it != mViews->end(); ++it)
		(*it)->puzzleShieldDidDeploy(tileIndex);
	mViewsLocked = false;
}

void Puzzle::puzzleShieldWasWithdrawn(int tileIndex)
{
	mViewsLocked = true;
	for(PuzzleViews::iterator it = mViews->begin(); it != mViews->end(); ++it)
		(*it)->puzzleShieldWasWithdrawn(tileIndex);
	mViewsLocked = false;
}

void Puzzle::puzzleTilesShouldRotate(const int2d& tileIndexes)
{
	mViewsLocked = true;
	for(PuzzleViews::iterator it = mViews->begin(); it != mViews->end(); ++it)
		(*it)->puzzleTilesShouldRotate(tileIndexes);
	mViewsLocked = false;
}

void Puzzle::puzzleTileSwapWillBegin(const int2d& swapIndexes, bool isCenterValid)
{
	mViewsLocked = true;
	for(PuzzleViews::iterator it = mViews->begin(); it != mViews->end(); ++it)
		(*it)->puzzleTileSwapWillBegin(swapIndexes, isCenterValid);
	mViewsLocked = false;
}

void Puzzle::puzzleConveyorBeltWillMove(const Coord& moveDir, int wrapIndex, const int1d* tileIndexes, int numTiles)
{
	mViewsLocked = true;
	for(PuzzleViews::iterator it = mViews->begin(); it != mViews->end(); ++it)
		(*it)->puzzleConveyorBeltWillMove(moveDir, wrapIndex, *tileIndexes, numTiles);
	mViewsLocked = false;
}

void Puzzle::puzzleWasSolved(int tileIndex)
{
	mViewsLocked = true;
	for(PuzzleViews::iterator it = mViews->begin(); it != mViews->end(); ++it)
		(*it)->puzzleWasSolved(tileIndex);
	mViewsLocked = false;
}

void Puzzle::processPrevMovements(void)
{
	if (mPaused)
        return;

	CCObject* obj;
	CCARRAY_FOREACH_REVERSE(mPlayers, obj)
	{
		Player* player = static_cast<Player*>(obj);
		if (player)
		{
			if (player->getDidMove())
				playerMoved(player);
			player->setDidMove(false);
		}
	}

    recolorPlayers(); // So that clones will receive last-minute tile color changes before setting their data to null.
    processQueuedCommands();
    recolorPlayers(); // So that players can adjust their color based on queued commands.

	CCARRAY_FOREACH(mPlayers, obj)
	{
		HumanPlayer* humanPlayer = dynamic_cast<HumanPlayer*>(obj);
		if (humanPlayer && humanPlayer->getMirroredSelf() && humanPlayer->getMirroredSelf()->getHasExpired())
        {
            removePlayer(humanPlayer->getMirroredSelf());
            humanPlayer->setMirroredSelf(NULL);
        }
	}
}

void Puzzle::processNextMovements(void)
{
    if (mPaused)
        return;
    
	CCObject* obj;
	CCARRAY_FOREACH_REVERSE(mPlayers, obj)
	{
		Player* player = static_cast<Player*>(obj);
		if (player && !player->isMoving())
		{
            tryMove(player);
			player->setQueuedMove(CMCoordZero);
		}
	}

    recolorPlayers();
}

void Puzzle::pause(void)
{
	mPaused = true;
}

void Puzzle::resume(void)
{
	mPaused = false;
}

void Puzzle::reset(void)
{
    mResetting = true;
    
	for (int i = 0; i < kQueuedCommandsCapacity; ++i)
    {
        if (mQueuedCommands[i])
        {
            mQueuedCommands[i]->setFunction(0);
            mQueuedCommands[i] = NULL;
        }
        else
            break;
    }

    if (mTiles)
    {
		int numTiles = getNumTiles();
		for (int i = 0; i < numTiles; ++i)
            mTiles[i].reset();
    }

    if (mPlayers)
    {
		CCObject* obj;
		CCArray* players = CCArray::createWithCapacity(mPlayers->count());
		CCARRAY_FOREACH(mPlayers, obj)
			players->addObject(obj);

		CCARRAY_FOREACH(players, obj)
		{
			Player* player = static_cast<Player*>(obj);
			if (player)
			{
				MirroredPlayer* mirroredPlayer = dynamic_cast<MirroredPlayer*>(player);
				if (mirroredPlayer)
					removePlayer(mirroredPlayer);
                else
                    player->reset();
			}
		}
    }

    resetTileSwapIndexes();
    mShouldSerialize = false;
    mSolved = false;
	setIsTileSwapping(false);
	setIsRotating(false);
	setIsConveyorBeltActive(false);
    
    mResetting = false;
}

void Puzzle::wipeClear(void)
{
	for (int i = 0, n = getNumTiles(); i < n; ++i)
	{
		mTiles[i].setDevProperties(0);
		mTiles[i].setDevPainter(0);
		mTiles[i].setModifiers(0);
	}
}

void Puzzle::resetTileSwapIndexes(void)
{
	for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
			mTileSwapIndexes.at(0, 3 * i + j) = -1;
			mTileSwapIndexes.at(1, 3 * i + j) = -1;
        }
    }
}

CCArray* Puzzle::getHumanPlayers(void) const
{
	CCArray* humanPlayers = CCArray::createWithCapacity(mPlayers->count());
	CCObject* obj;
	CCARRAY_FOREACH(mPlayers, obj)
	{
		Player* player = static_cast<Player*>(obj);
		if (player && player->getType() == Player::HUMAN_PLAYER)
			humanPlayers->addObject(player);
	}

	return humanPlayers;
}

Puzzle* Puzzle::clone(void) const
{
	Puzzle* puzzle = new Puzzle(getID(), getName(), getNumColumns(), getNumRows());
	puzzle->autorelease();
    puzzle->setIQ(getIQ());
    
    if (mTiles)
    {
		int numTiles = puzzle->getNumTiles();
        for (int i = 0; i < numTiles; ++i)
			puzzle->mTiles[i].copyProperties(mTiles[i]);
    }

    if (mPlayers)
    {
		CCObject* obj;
		CCARRAY_FOREACH(mPlayers, obj)
		{
			Player* player = static_cast<Player*>(obj);
			if (player && player->getType() == Player::HUMAN_PLAYER)
				puzzle->mPlayers->addObject(player->clone());
		}
    }

    return puzzle;
}

Puzzle* Puzzle::devClone(void) const
{
	Puzzle* puzzle = new Puzzle(getID(), getName(), getNumColumns(), getNumRows());
	puzzle->autorelease();
    puzzle->setIQ(getIQ());

    if (mTiles)
    {
		int numTiles = puzzle->getNumTiles();
        for (int i = 0; i < numTiles; ++i)
			puzzle->mTiles[i].copyDevProperties(mTiles[i]);
    }

    if (mPlayers)
    {
		CCObject* obj;
		CCARRAY_FOREACH(mPlayers, obj)
		{
			Player* player = static_cast<Player*>(obj);
			if (player && player->getType() == Player::HUMAN_PLAYER)
				puzzle->mPlayers->addObject(player->devClone());
		}
    }

    return puzzle;
}


/////////////////////////////////////////////////////////////
//--------------------- Serialization ---------------------//
/////////////////////////////////////////////////////////////


typedef struct TileData
{
    TileData(void)
    : _properties(0), _modifiers(0), _painter(0)
    { }
    explicit TileData(uint properties, uint modifiers, uint painter)
    : _properties(properties), _modifiers(modifiers), _painter(painter)
    { }

    uint _properties;
    uint _modifiers;
    uint _painter;
} TileData;

typedef struct PlayerData
{
    PlayerData(void)
    : _orientation(0), _colorKey(0), _numColorMagicMoves(0), _numMovesRemaining(0), _position(CMCoordZero)
    { }
    explicit PlayerData(uint orientation, uint colorKey, int numColorMagicMoves, int numMovesRemaining, Coord position)
    : _orientation(orientation), _colorKey(colorKey), _numColorMagicMoves(numColorMagicMoves), _numMovesRemaining(numMovesRemaining), _position(position)
    { }
    
    uint _orientation;
    uint _colorKey;
    int _numColorMagicMoves;
    int _numMovesRemaining;
    Coord _position;
} PlayerData;

static const int kSerializeDataVersion = 1;
void deserializeCallback(void* arg)
{
    if (arg == NULL)
        return;
    
    FMLoadData* loadData = (FMLoadData*)arg;
    ulong size = loadData->_size, offset = loadData->_offset;
    CCAssert(size >= offset, "Invalid file stream pointers in Puzzle::deserializeCallback.");
    
    Puzzle* puzzle = (Puzzle*)loadData->_data;
    u8* stream = loadData->_stream;
    CCAssert(puzzle && stream, "Invalid user data in Puzzle::deserializeCallback.");
    
#if CM_BETA
    CCLog("*************************");
    CCLog("Deserialize size: %d", (int)size);
    CCLog("*************************");
#endif
    
    do
    {
        bool innerBreak = false;
        loadData->_error = true;
        
        if (puzzle == NULL || stream == NULL || size < offset)
            break;
        
        stream += offset;
        
        // Header
        int dataVersion = -1;
        if ((size - offset) >= sizeof(dataVersion))
        {
            dataVersion = *(int*)(stream + offset);
            offset += sizeof(dataVersion);
        }
        else
            break;
        
        int numColumns = -1, numRows = -1;
        if ((size - offset) >= sizeof(numColumns) + sizeof(numRows))
        {
            numColumns = *(int*)(stream + offset);
            offset += sizeof(numColumns);
            numRows = *(int*)(stream + offset);
            offset += sizeof(numRows);
            
            if (numColumns != puzzle->getNumColumns() || numRows != puzzle->getNumRows())
                break;
        }
        else
            break;
        
        // Tiles
        TileData tileData;
        if ((size - offset) < numColumns * numRows * sizeof(tileData))
            break;
        
        std::vector<TileData> tileDataCollection;
        tileDataCollection.reserve(numColumns * numRows);
        for (int i = 0; i < numRows && !innerBreak; ++i)
        {
            for (int j = 0; j < numColumns && !innerBreak; ++j)
            {
                int index = i * numColumns + j;
                if (!puzzle->isValidIndex(index))
                {
                    innerBreak = true;
                    break;
                }
                
                tileData = *(TileData*)(stream + offset);
                offset += sizeof(tileData);
                tileDataCollection.push_back(tileData);
            }
        }
        
        // Players
        int numPlayers = -1;
        if ((size - offset) >= sizeof(numPlayers))
        {
            numPlayers = *(int*)(stream + offset);
            offset += sizeof(numPlayers);
            
            if (numPlayers <= 0 || numPlayers > 2)
                break;
        }
        else
            break;
        
        PlayerData playerData;
        if ((size - offset) < numPlayers * sizeof(playerData))
            break;
        
        std::vector<PlayerData> playerDataCollection;
        playerDataCollection.reserve(numPlayers);
        for (int i = 0; i < numPlayers && !innerBreak; ++i)
        {
            playerData = *(PlayerData*)(stream + offset);
            offset += sizeof(playerData);
            playerDataCollection.push_back(playerData);
        }
        
        ////////////////////////////////////////
        //-------- Apply loaded state --------//
        ////////////////////////////////////////
        
        HumanPlayer* humanPlayer = puzzle->getAnyHumanPlayer();
        if (humanPlayer == NULL || humanPlayer->getMirroredSelf())
            break;
        
        // Apply Tile states
        int tileIndex = 0;
        for (std::vector<TileData>::iterator it = tileDataCollection.begin(); it != tileDataCollection.end(); ++it, ++tileIndex)
        {
            PuzzleTile* tile = puzzle->tileAtIndex(tileIndex);
            TileData tileData = *it;
            tile->setProperties(tileData._properties);
            tile->setModifiers(tileData._modifiers);
            tile->setPainter(tileData._painter);
        }
        
        // Apply Player states
        int playerIndex = 0;
        for (std::vector<PlayerData>::iterator it = playerDataCollection.begin(); it != playerDataCollection.end(); ++it, ++playerIndex)
        {
            PlayerData playerData = *it;
            if (playerIndex == 0)
            {
                humanPlayer->setOrientation(playerData._orientation);
                humanPlayer->setColorKey(playerData._colorKey);
                humanPlayer->setNumColorMagicMoves(playerData._numColorMagicMoves);
                humanPlayer->setPosition(playerData._position);
            }
            else if (playerData._numMovesRemaining > 0)
            {
                MirroredPlayer* mirroredPlayer = MirroredPlayer::create(playerData._colorKey, playerData._position, playerData._orientation);
                humanPlayer->setMirroredSelf(mirroredPlayer);
                puzzle->addPlayer(mirroredPlayer);
                mirroredPlayer->setNumMovesRemaining(playerData._numMovesRemaining);
            }
        }
        
        // Do after players are added/positioned so that the PuzzleBoard can better sort Z-values.
        tileIndex = 0;
        for (std::vector<TileData>::iterator it = tileDataCollection.begin(); it != tileDataCollection.end(); ++it, ++tileIndex)
        {
            PuzzleTile* tile = puzzle->tileAtIndex(tileIndex);
            if (tile->getFunctionKey() == PuzzleTile::kTFShield && tile->isModified(PuzzleTile::kTFShield))
                puzzle->puzzleShieldDidDeploy(tileIndex);
        }

        loadData->_error = false;
    } while(false);
    
    loadData->_offset = offset;
    
    if (!loadData->_error)
        puzzle->mWasSerializationSuccessful = true;
}

void serializeCallback(void* arg)
{
    if (arg == NULL)
        return;
    
    FMSaveData* fmSaveData = (FMSaveData*)arg;
    Puzzle* puzzle = (Puzzle *)fmSaveData->_data;
    
    do
    {
        bool innerBreak = false;
        fmSaveData->_error = true;
        
        FILE* file = fmSaveData->_file;
        if (file == NULL || puzzle == NULL)
            break;
        
        // Header
        if (fwrite(&kSerializeDataVersion, sizeof(kSerializeDataVersion), 1, file) != 1)
            break;
        puzzle->mSerializeWriteCount += sizeof(kSerializeDataVersion);
        
        int numColumns = puzzle->getNumColumns(), numRows = puzzle->getNumRows();
        if (fwrite(&numColumns, sizeof(numColumns), 1, file) != 1 || fwrite(&numRows, sizeof(numRows), 1, file) != 1)
            break;
        puzzle->mSerializeWriteCount += sizeof(numColumns) + sizeof(numRows);
        
        // Tiles
        std::vector<TileData> tileDataCollection;
        tileDataCollection.reserve(numColumns * numRows);
        for (int i = 0; i < numRows && !innerBreak; ++i)
        {
            for (int j = 0; j < numColumns && !innerBreak; ++j)
            {
                int index = i * numColumns + j;
                if (!puzzle->isValidIndex(index))
                {
                    innerBreak = true;
                    break;
                }
                
                PuzzleTile* tile = puzzle->tileAtIndex(index);
                tileDataCollection.push_back(TileData(tile->getProperties(), tile->getModifiers(), tile->getPainter()));
            }
        }
        
        if (fwrite(tileDataCollection.data(), sizeof(TileData), tileDataCollection.size(), file) != tileDataCollection.size())
            break;
        puzzle->mSerializeWriteCount += sizeof(TileData) * tileDataCollection.size();
        
        // Players
        HumanPlayer* humanPlayer = puzzle->getAnyHumanPlayer();
        if (humanPlayer == NULL)
            break;
        
        std::vector<Player*> players;
        players.push_back(humanPlayer);
        
        int numMirroredMovesRemaining = 0;
        MirroredPlayer* mirroredPlayer = humanPlayer->getMirroredSelf();
        if (mirroredPlayer)
        {
            numMirroredMovesRemaining = mirroredPlayer->getNumMovesRemaining();
            
            if (numMirroredMovesRemaining > 0)
                players.push_back(mirroredPlayer);
        }
        
        int numPlayers = (int)players.size();
        if (fwrite(&numPlayers, sizeof(numPlayers), 1, file) != 1)
            break;
        puzzle->mSerializeWriteCount += sizeof(numPlayers);
        
        std::vector<PlayerData> playerDataCollection;
        playerDataCollection.reserve(numPlayers);
        for (std::vector<Player*>::iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = *it;
            playerDataCollection.push_back(PlayerData(
                                                      player->isMoving() ? player->getPrevOrientation() : player->getOrientation(),
                                                      player->getColorKey(),
                                                      player->getNumColorMagicMoves(),
                                                      player == mirroredPlayer ? numMirroredMovesRemaining : 0,
                                                      player->getPosition()));
        }
        
        if (fwrite(playerDataCollection.data(), sizeof(PlayerData), playerDataCollection.size(), file) != playerDataCollection.size())
            break;
        puzzle->mSerializeWriteCount += sizeof(PlayerData) * playerDataCollection.size();
        
        CCLog("-- Serialized all %d bytes --", puzzle->mSerializeWriteCount);
        fmSaveData->_error = false;
    } while (false);
    
    if (!fmSaveData->_error)
        puzzle->mWasSerializationSuccessful = true;
}

bool Puzzle::deserializePuzzleState(const char* filepath)
{
    if (getSerializeState() != Puzzle::SERIALIZE_NONE || filepath == NULL)
        return false;
    
    setSerializeState(SERIALIZE_LOAD);
    FileManager::FM()->load(filepath, this, deserializeCallback);
    setSerializeState(SERIALIZE_NONE);
    
    mShouldSerialize = mWasSerializationSuccessful;
    return mWasSerializationSuccessful;
}

bool Puzzle::serializePuzzleState(const char* filepath)
{
    if (getSerializeState() != Puzzle::SERIALIZE_NONE || filepath == NULL || mSolved || !mShouldSerialize)
        return false;
    
    dispatchEvent(EV_TYPE_FORCE_COMPLETE_ALL_MECHANICS(), this);
    
    setSerializeState(SERIALIZE_SAVE);
    FileManager::FM()->save(filepath, this, serializeCallback);
    setSerializeState(SERIALIZE_NONE);
    
    //mShouldSerialize = !mWasSerializationSuccessful;
    return mWasSerializationSuccessful;
}

const size_t kSerializeBufferCount = 2048; // Note: serialization write is 1000 bytes in length @version 1.0
std::string Puzzle::getCurrentPuzzleSerialized()
{
    if (serializeBuffer == NULL)
        serializeBuffer = (unsigned char* )malloc(kSerializeBufferCount * sizeof(unsigned char));
    memset(serializeBuffer, 0, kSerializeBufferCount * sizeof(unsigned char)); // Ensure clients' possible string conversions are null terminated.
    
    dispatchEvent(EV_TYPE_FORCE_COMPLETE_ALL_MECHANICS(), this);
    
    sXPuzzle sxpuzzle;
    memset(sxpuzzle.name, 0, ksXPuzzleNameLen * sizeof(char));
    memcpy(sxpuzzle.name, mName.c_str(), MIN(ksXPuzzleNameLen-1, mName.length()));
    
    bool wasSerializationSuccessfulCache = mWasSerializationSuccessful;
    setSerializeState(SERIALIZE_SAVE);
    FileManager::FM()->write(this, serializeBuffer, kSerializeBufferCount, serializeCallback);
    setSerializeState(SERIALIZE_NONE);
    mWasSerializationSuccessful = wasSerializationSuccessfulCache; // Restore previous state for normal serialization functionality.
    
    // Convert to comma separated list of values
    std::ostringstream oss;
    int len = MIN(kSerializeBufferCount, mSerializeWriteCount);
    if (len > 0)
    {
        std::string comma = ",";
        oss << getID() << comma;
        oss << (const char*)sxpuzzle.name << comma;
     
        for (int i = 0; i < len; ++i)
        {
            if (i < len-1)
                oss << (uint)serializeBuffer[i] << comma;
            else
                oss << (uint)serializeBuffer[i];
        }
    }
    
    return oss.str();
}
