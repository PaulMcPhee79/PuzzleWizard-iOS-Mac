
#include "PuzzleTile.h"
#include <Utils/PWDebug.h>
USING_NS_CC;

typedef std::set<ITileView*> TileViews;

PuzzleTile::PuzzleTile(void)
	:
mViewsLocked(false),
mIsEdgeTile(false),
mViews(NULL),
mDecorator(0),
mDecoratorData(0),
mProperties(0),
mModifiers(0),
mPainter(0),
mDevProperties(0),
mDevPainter(0),
mGUID(1)
{
    PWDebug::tileCount++;
}

PuzzleTile::~PuzzleTile(void)
{
	delete mViews, mViews = NULL;
}

PuzzleTile::PuzzleTile(const PuzzleTile& other)
{
	copyHelper(other);
}

PuzzleTile& PuzzleTile::operator=(const PuzzleTile& rhs)
{
	if (this != &rhs)
	{
		PuzzleTile tmp(rhs);
		delete mViews, mViews = NULL;
		copyHelper(tmp);
	}
	return *this;
}

PuzzleTile* PuzzleTile::create(bool autorelease)
{
    PuzzleTile *tile = new PuzzleTile();
    if (tile)
    {
		if (autorelease)
			tile->autorelease();
        return tile;
    }
    CC_SAFE_DELETE(tile);
    return NULL;
}

void PuzzleTile::copyHelper(const PuzzleTile& other)
{
	setDevProperties(other.getDevProperties());
	setDevPainter(other.getDevPainter());
	setProperties(other.getProperties());
	setModifiers(other.getModifiers());
	setPainter(other.getPainter());
	mGUID = other.mGUID;
	mViewsLocked = false;
	mViews = other.mViews ? new TileViews(*other.mViews) : NULL;
}

PuzzleTile PuzzleTile::propertyClone()
{
	PuzzleTile tile;
	tile.setProperties(getProperties());
	tile.setModifiers(getModifiers());
	tile.setPainter(getPainter());
	return tile;
}

PuzzleTile PuzzleTile::devPropertyClone()
{
	PuzzleTile tile;
	tile.setDevProperties(getDevProperties());
	tile.setModifiers(getModifiers());
	tile.setDevPainter(getDevPainter());
	return tile;
}

void PuzzleTile::notifyPropertyChange(uint code)
{
	mViewsLocked = true;
	if (mViews && mViews->size() > 0)
	{
		for(TileViews::iterator it = mViews->begin(); it != mViews->end(); ++it)
			(*it)->tilePropertiesDidChange(code);
	}
	mViewsLocked = false;
}

void PuzzleTile::notifyModiferChange()
{
	mViewsLocked = true;
	if (mViews && mViews->size() > 0)
	{
		for(TileViews::iterator it = mViews->begin(); it != mViews->end(); ++it)
			(*it)->tileModifiersDidChange();
	}
	mViewsLocked = false;
}

void PuzzleTile::registerView(ITileView* view)
{
	CCAssert(!mViewsLocked, "ITileView is immutable during event propagation.");
	if (mViews == NULL)
		mViews = new TileViews();
	if (view)
		mViews->insert(view);
}

void PuzzleTile::deregisterView(ITileView* view)
{
	CCAssert(!mViewsLocked, "ITileView is immutable during event propagation.");
	if (mViews && view)
		mViews->erase(view);
}

void PuzzleTile::clearViews(void)
{
	CCAssert(!mViewsLocked, "ITileView is immutable during event propagation.");
	if (mViews)
		mViews->clear();
}

void PuzzleTile::modifyPainter(uint painter)
{
	for (int i = 0; i < 4; ++i)
    {
        uint mask = (uint)(0xf << (i * 4));
        if ((painter & mask) != 0)
        {
            mPainter &= ~mask;
            mPainter |= painter & mask;
        }
    }

    setPainter(mPainter);
}

void PuzzleTile::devModifyPainter(uint painter)
{
	for (int i = 0; i < 4; ++i)
    {
        uint mask = (uint)(0xf << (i * 4));
        if ((painter & mask) != 0)
        {
            mDevPainter &= ~mask;
            mDevPainter |= painter & mask;
        }
    }

    setDevPainter(mDevPainter);
}

void PuzzleTile::copyProperties(const PuzzleTile& other)
{
	setProperties(other.getProperties());
	setModifiers(other.getModifiers());
	setPainter(other.getPainter());
}

void PuzzleTile::copyDevProperties(const PuzzleTile& other)
{
	setDevProperties(other.getDevProperties());
	setModifiers(other.getModifiers());
	setDevPainter(other.getDevPainter());
}

void PuzzleTile::clear()
{
	setProperties(0);
	setModifiers(0);
	setPainter(0);
}

void PuzzleTile::reset()
{
	setProperties(getDevProperties());
	setModifiers(0);
	setPainter(getDevPainter());
}
