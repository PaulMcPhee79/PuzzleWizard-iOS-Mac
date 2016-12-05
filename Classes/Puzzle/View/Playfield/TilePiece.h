#ifndef __TILE_PIECE_H__
#define __TILE_PIECE_H__

#include "cocos2d.h"
#include <Prop/CMSprite.h>
#include <Interfaces/ITileView.h>
#include <Interfaces/IReusable.h>
class ReusableCache;
class PuzzleTile;
class TileDecoration;
class SpriteColorer;
class ITileDecorator;
USING_NS_CC;

class TilePiece : public CMSprite, public ITileView, public IReusable
{
public:
	enum AestheticState { NORMAL, EDGE, OCCLUSION };

	// Tile color keys
    static const uint kColorKeyNone = 0;
    static const uint kColorKeyRed = 1;
    static const uint kColorKeyBlue = 2;
    static const uint kColorKeyGreen = 3;
    static const uint kColorKeyYellow = 4;
    static const uint kColorKeyWhite = 5;
    static const uint kColorKeyMulti = 6;

    // Decoration keys
    static const uint kTDKTeleport = 0x10;
    static const uint kTDKColorSwap = 0x20;
    static const uint kTDKRotate = 0x30;
    static const uint kTDKShield = 0x40;
    static const uint kTDKPainter = 0x50;
    static const uint kTDKTileSwap = 0x60;
    static const uint kTDKMirroredImage = 0x70;
    static const uint kTDKKey = 0x80;
    static const uint kTDKColorFill = 0x90;
    static const uint kTDKPaintBomb = 0xa0;
    static const uint kTDKConveyorBelt = 0xb0;
    static const uint kTDKColorMagic = 0xc0;

    // Decoration style key ranges
    static const uint kTDSKTeleport = 4;
    static const uint kTDSKColorSwap = 10; // rb,rg,ry,rw,bg,by,bw,gy,gw,yw

	TilePiece(PuzzleTile* tile);
	virtual ~TilePiece(void);

	static TilePiece* create(PuzzleTile* tile, bool autorelease = true);
	virtual bool init(void);
	virtual TilePiece* clone();

	static TilePiece* getTilePiece(PuzzleTile* tile);
	static void setupReusables(void);
    virtual void returnToPool(void);

    void updateMenuScale(void);
	void enableMenuMode(bool enable);
    void refreshAesthetics(void);

	void enablePath(bool colorMagic);
	void setPathOpacity(GLubyte value);
	void disablePath(void);

    PuzzleTile* getData() { return mTile; }
	void setData(PuzzleTile* tile);
	void syncWithData(void);
	void syncWithTilePiece(TilePiece* tilePiece);

	virtual void tilePropertiesDidChange(uint code);
	virtual void tileModifiersDidChange(void)  { syncWithData(); }

	bool isBatchable(void) const;

	float getOffsetY(void) const;
	CCPoint getShadowPosition(void);
	virtual void setAestheticState(AestheticState value);
	void setTeleportOpacity(GLubyte value);
	CCSize tileDimensions(void) const;
	CCRect tileBounds(void);

	ITileDecorator* getDecorator(void) const { return mDecorator; }
	void setDecorator(ITileDecorator* value);

	static void setColorKeysForSwapKey(uint swapKey, uint& colorKeyLeft, uint& colorKeyRight);

protected:
	void setTile(PuzzleTile* value);
	void transitionToColor(const ccColor3B& color, float duration = kColorTransitionDuration, float delay = 0.0f);
	AestheticState getAestheticState(void) { return mAesState; }
	void updateEdgeStatus(void);
    float getMenuModeTileScale(void) const;
    void setPathDisplayFrame(uint colorKey, bool colorMagic);

private:
	static const float kColorTransitionDuration;

    virtual int getReuseKey(void) { return 0; }
	virtual void reuse(void);
    virtual void hibernate(void);
	static IReusable* checkoutReusable(uint reuseKey);
	static void checkinReusable(IReusable* reusable);

	static ReusableCache* sCache;
    static bool sCaching;

	TileDecoration* getDecoration(void) const { return mDecoration; }
	void setDecoration(TileDecoration* value);

	AestheticState mAesState;
    PuzzleTile* mTile;
    TileDecoration* mDecoration;
	ITileDecorator* mDecorator;
    SpriteColorer* mTileColorer;

	bool mColorMagicActive;
    bool mMenuModeEnabled;
	CCSprite* mPathIcon;
};
#endif // __TILE_PIECE_H__
