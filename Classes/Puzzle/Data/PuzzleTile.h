#ifndef __PUZZLE_TILE_H__
#define __PUZZLE_TILE_H__

#include "cocos2d.h"
#include <Interfaces/ITileView.h>
USING_NS_CC;

class PuzzleTile : public CCObject
{
public:
	// Tile Functions
    static const uint kTFNone = 0;
    static const uint kTFTeleport = 1 << 16;
    static const uint kTFColorSwap = 2 << 16;
    static const uint kTFRotate = 3 << 16;
    static const uint kTFShield = 4 << 16;
    static const uint kTFPainter = 5 << 16;
    static const uint kTFTileSwap = 6 << 16;
    static const uint kTFMirroredImage = 7 << 16;
    static const uint kTFKey = 8 << 16;
    static const uint kTFColorFill = 9 << 16;
    static const uint kTFPaintBomb = 10 << 16;
    static const uint kTFConveyorBelt = 11 << 16;
    static const uint kTFColorMagic = 12 << 16;

	// Property masks
    static const uint kColorKeyMask = 0xf;
    static const uint kDecorationKeyMask = 0xff0;
    static const uint kDecorationStyleKeyMask = 0xf000;
    static const uint kFunctionKeyMask = 0xff0000;
    static const uint kFunctionIDMask = 0xff000000;

    // Bit shifts
    static const int kBitShiftColor = 0;
    static const int kBitShiftDecoration = 4;
    static const int kBitShiftDecorationStyle = 12;
    static const int kBitShiftFunction = 16;
    static const int kBitShiftFunctionID = 24;
	
	// Extended Modifiers (can be set alongside Tile Function modifiers)
	static const uint kTMShielded = 1 << 0;

	// Property change codes
	static const uint kPropAll = 0;
    static const uint kPropCodeIsEdge = 1;

	PuzzleTile(void);
	virtual ~PuzzleTile(void);

    static PuzzleTile* create(bool autorelease = true);

	PuzzleTile(const PuzzleTile& other);
	PuzzleTile& operator=(const PuzzleTile& rhs);
	PuzzleTile propertyClone();
	PuzzleTile devPropertyClone();

	void registerView(ITileView* view);
	void deregisterView(ITileView* view);
	void clearViews(void);
	bool isEdgeTile(void) const { return mIsEdgeTile; }
	void setEdgeTile(bool value)
	{
		if (mIsEdgeTile != value)
		{
			mIsEdgeTile = value;
			notifyPropertyChange(kPropCodeIsEdge);
		}
	}
	bool isModified(uint modifier) { return (mModifiers & modifier) == modifier; }
	void modifyPainter(uint painter);
	void devModifyPainter(uint painter);
	void copyProperties(const PuzzleTile& other);
	void copyDevProperties(const PuzzleTile& other);
	void clear();
	void reset();

	uint getColorKey(void) const { return mProperties & kColorKeyMask; }
	void setColorKey(uint value) { setProperties((mProperties & ~kColorKeyMask) | (value & kColorKeyMask)); }
	uint getDecorationKey(void) const { return mProperties & kDecorationKeyMask; }
	void setDecorationKey(uint value) { setProperties((mProperties & ~kDecorationKeyMask) | (value & kDecorationKeyMask)); }
	uint getDecorationStyleKey(void) const { return mProperties & kDecorationStyleKeyMask; }
	void setDecorationStyleKey(uint value) { setProperties((mProperties & ~kDecorationStyleKeyMask) | (value & kDecorationStyleKeyMask)); }
	uint getFunctionKey(void) const { return mProperties & kFunctionKeyMask; }
	void setFunctionKey(uint value) { setProperties((mProperties & ~kFunctionKeyMask) | (value & kFunctionKeyMask)); }
	uint getFunctionID(void) const { return mProperties & kFunctionIDMask; }
	void setFunctionID(uint value) { setProperties((mProperties & ~kFunctionIDMask) | (value & kFunctionIDMask)); }
	uint getDevColorKey(void) const { return mDevProperties & kColorKeyMask; }
	void setDevColorKey(uint value) { setDevProperties((mDevProperties & ~kColorKeyMask) | (value & kColorKeyMask)); }
	uint getDevDecorationKey(void) const { return mDevProperties & kDecorationKeyMask; }
	void setDevDecorationKey(uint value) { setDevProperties((mDevProperties & ~kDecorationKeyMask) | (value & kDecorationKeyMask)); }
	uint getDevDecorationStyleKey(void) const { return mDevProperties & kDecorationStyleKeyMask; }
	void setDevDecorationStyleKey(uint value) { setDevProperties((mDevProperties & ~kDecorationStyleKeyMask) | (value & kDecorationStyleKeyMask)); }
	uint getDevFunctionKey(void) const { return mDevProperties & kFunctionKeyMask; }
	void setDevFunctionKey(uint value) { setDevProperties((mDevProperties & ~kFunctionKeyMask) | (value & kFunctionKeyMask)); }
	uint getDevFunctionID(void) const { return mDevProperties & kFunctionIDMask; }
	void setDevFunctionID(uint value) { setDevProperties((mDevProperties & ~kFunctionIDMask) | (value & kFunctionIDMask)); }
	uint getDecorator(void) const { return mDecorator; }
	void setDecorator(uint value) { mDecorator = value; }
	int getDecoratorData(void) const { return mDecoratorData; }
	void setDecoratorData(int value) { mDecoratorData = value; }
	uint getGUID(void) const { return mGUID; }
	void setGUID(uint value) { mGUID = value; }
	uint getProperties(void) const { return mProperties; }
	void setProperties(uint value)
	{
		mProperties = value;
		notifyPropertyChange();
        setDecorator(0);
        setDecoratorData(0);
	}
	uint getModifiers(void) const { return mModifiers; }
	void setModifiers(uint value)
	{
		mModifiers = value;
		notifyModiferChange();
	}
	uint getPainter(void) const { return mPainter; }
	void setPainter(uint value)
	{
		mPainter = value;
		notifyModiferChange();
	}

	uint getDevProperties(void) const { return mDevProperties; }
	void setDevProperties(uint value)
	{
		mDevProperties = value;
		setProperties(value);
	}
	uint getDevPainter(void) const { return mDevPainter; }
	void setDevPainter(uint value)
	{
		mDevPainter = value;
		setPainter(value);
	}

protected:
	virtual void copyHelper(const PuzzleTile& other);

private:
	void notifyPropertyChange(uint code = kPropAll);
	void notifyModiferChange();

	bool mIsEdgeTile;
	uint mDecorator;
    int mDecoratorData;
    uint mGUID;

    // [0-3] Color, [4-11] Decoration, [12-15] Decoration Style [16-23] Function, [24-31] Function ID
    uint mProperties;
    uint mModifiers;
    // [0-3] North color, [4-7] East color, [8-11] South color, [12-15] West color
    uint mPainter;

    uint mDevProperties;
    uint mDevPainter;

	bool mViewsLocked;
	std::set<ITileView*>* mViews;
};
#endif // __PUZZLE_TILE_H__
