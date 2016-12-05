
#include "ShieldManager.h"
#include <Utils/Utils.h>
USING_NS_CC;

ShieldManager::ShieldManager(int category, int numColumns, int numRows, const CCRect& viewableRegion)
	:
CroppedProp(category, viewableRegion),
mNumColumns(numColumns),
mNumRows(numRows)
{

}

ShieldManager::~ShieldManager(void)
{
	for (std::map<int, Shield*>::iterator it = mShields.begin(); it != mShields.end(); ++it)
	{
		Shield* shield = (*it).second;
		shield->setListener(NULL);
		shield->returnToPool();
		CC_SAFE_RELEASE(shield);
	}

	mShields.clear();
}

bool ShieldManager::isIndexSurroundedBy(int index, int other, int range) const
{
	int indexColumn = columnForIndex(index), indexRow = rowForIndex(index);
    int otherColumn = columnForIndex(other), otherRow = rowForIndex(other);
    return otherColumn >= 0 && otherColumn < mNumColumns && otherRow >= 0 && otherRow < mNumRows
		&& abs(indexColumn - otherColumn) <= range && abs(indexRow - otherRow) <= range;
}

int ShieldManager::rowForIndex(int index) const
{
	return index / mNumColumns;
}

int ShieldManager::columnForIndex(int index) const
{
	return index % mNumColumns;
}

int ShieldManager::calcRowDelta(int index, int other) const
{
	return rowForIndex(other) - rowForIndex(index);
}

int ShieldManager::calcColumnDelta(int index, int other) const
{
	return columnForIndex(other) - columnForIndex(index);
}

void ShieldManager::updateFoci(void)
{
    for (std::map<int, Shield*>::iterator itOuter = mShields.begin(); itOuter != mShields.end(); ++itOuter)
    {
        Shield* shieldOuter = (*itOuter).second;
        if (!shieldOuter->isDeployed())
            continue;
        
        int numFociSet = 0;
        int outerTileIndex = shieldOuter->getTileIndex();
        
        shieldOuter->resetStencils();
        for (std::map<int, Shield*>::iterator itInner = mShields.begin(); itInner != mShields.end(); ++itInner)
        {
            Shield* shieldInner = (*itInner).second;
            if (shieldOuter == shieldInner || !shieldInner->isDeployed())
                continue;
            
            int innerTileIndex = shieldInner->getTileIndex();
            
            if (isIndexSurroundedBy(outerTileIndex, innerTileIndex, 1))
            {
                int rowDelta = calcRowDelta(outerTileIndex, innerTileIndex);
                int colDelta = calcColumnDelta(outerTileIndex, innerTileIndex);
                int rowXcol = rowDelta * colDelta;
                
                if (rowXcol == 0) // nesw
                {
                    shieldOuter->setStencil(numFociSet, 1, atan2f(-colDelta, -rowDelta));
                }
                else // diagonal
                {
                    shieldOuter->setStencil(numFociSet, 2, atan2f(-colDelta, -rowDelta));
                }
            }
            else if (isIndexSurroundedBy(outerTileIndex, innerTileIndex, 2))
            {
                int rowDelta = calcRowDelta(outerTileIndex, innerTileIndex);
                int colDelta = calcColumnDelta(outerTileIndex, innerTileIndex);
                int rowXcol = rowDelta * colDelta;
                
                if (rowXcol == 0) // nesw
                {
                    shieldOuter->setStencil(numFociSet, 3, atan2f(-colDelta, -rowDelta));
                }
                else // diagonal
                {
                    if (abs(rowXcol) == 4)
                        continue;
                    shieldOuter->setStencil(numFociSet, 4, atan2f(-colDelta, -rowDelta));
                }
            }
            else
                continue;
            
            if (++numFociSet == Shield::kMaxFoci)
                break;
        }
    }
}

Shield* ShieldManager::shieldForKey(int key)
{
	std::map<int, Shield*>::iterator it = mShields.find(key);
	return it != mShields.end() ? (*it).second : NULL;
}

Shield* ShieldManager::addShield(int key, int tileIndex, const CCPoint& shieldOrigin)
{
	Shield* duplicate = shieldForKey(key);
	if (duplicate)
	{
		duplicate->setID(-1); // Mark as expired
		duplicate->withdraw();
		mShields.erase(key);
		updateFoci();
	}

	Shield* shield = Shield::getShield(key, tileIndex, shieldOrigin);
	shield->retain();
	shield->setListener(this);
	mShields[key] = shield;
	addChild(shield);

	if (rowForIndex(tileIndex) == 0)
		shield->setType(Shield::TOP);
	else if (rowForIndex(tileIndex) == mNumRows-1)
		shield->setType(Shield::BOTTOM);
	else
		shield->setType(Shield::NORMAL);

	if (shield->isDeployed())
		updateFoci();
	return shield;
}

void ShieldManager::removeShield(int key)
{
	Shield* shield = shieldForKey(key);
	if (shield == NULL)
		return;

	shield->setListener(NULL);
	mShields.erase(key);

	shield->returnToPool();
	CC_SAFE_RELEASE_NULL(shield);

	updateFoci();
}

void ShieldManager::withdrawAll(bool playSound)
{
	std::map<int, Shield*> shields = mShields;
	for (std::map<int, Shield*>::iterator it = shields.begin(); it != shields.end(); ++it)
		(*it).second->withdraw(playSound);
}

void ShieldManager::onEvent(int evType, void* evData)
{
	if (evType == Shield::EV_TYPE_DEPLOYING())
	{
		updateFoci();
	}
	else if (evType == Shield::EV_TYPE_DEPLOYED())
	{

	}
	else if (evType == Shield::EV_TYPE_WITHDRAWING())
	{
		updateFoci();
	}
	else if (evType == Shield::EV_TYPE_WITHDRAWN())
	{
		Shield* shield = static_cast<Shield*>(evData);
		if (shield)
		{
			if (shield->getID() != -1)
				removeShield(shield->getID());
			else
			{
				// Duplicate - removed earlier.
                shield->returnToPool();
				CC_SAFE_RELEASE_NULL(shield);

                updateFoci();
			}
		}
	}
}

void ShieldManager::advanceTime(float dt)
{
    // Shield::advanceTime doesn't do anything that would alter the collection.
	for (std::map<int, Shield*>::iterator it = mShields.begin(); it != mShields.end(); ++it)
		(*it).second->advanceTime(dt);
}
