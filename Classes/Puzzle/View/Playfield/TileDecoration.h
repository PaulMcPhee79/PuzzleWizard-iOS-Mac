#ifndef __TILE_DECORATION_H__
#define __TILE_DECORATION_H__

#include "cocos2d.h"
#include <Prop/CMSprite.h>
#include <Interfaces/IReusable.h>
class ReusableCache;
class ByteTweener;
class ITileDecorator;
class Twinkle;
USING_NS_CC;

class TileDecoration : public CMSprite, public IReusable
{
public:
	static const GLubyte kTeleportGlowOpacityMin;
	static const GLubyte kTeleportGlowOpacityMax;
	static const float kTeleportGlowDuration;
    static const float kColorArrowFps;

	TileDecoration(uint type, uint subType);
	virtual ~TileDecoration(void);
	
	static TileDecoration* create(uint type, uint subType, bool autorelease = true);
	virtual bool init(void);

	static TileDecoration* getTileDecoration(uint type, uint subType);
	static void setupReusables(void);
    virtual void returnToPool(void);

	virtual void advanceTime(float dt);
	virtual void draw(void);

	uint getType(void) { return mType; }
	uint getSubType(void) { return mSubType; }
	void setSubType(uint value);
	GLubyte getTeleportOpacity(void);
	void setTeleportOpacity(GLubyte value);
    void setColorArrowFrameIndex(int value);

    void enableUIMode(bool enable);
	void enableMenuMode(bool enable);
	void syncWithTileDecoration(TileDecoration* tileDecoration);

	ITileDecorator* getDecorator(void) const { return mDecorator; }
	void setDecorator(ITileDecorator* value);

private:
    virtual int getReuseKey(void) { return getType(); }
	virtual void reuse(void);
    virtual void hibernate(void);
	static IReusable* checkoutReusable(uint reuseKey);
	static void checkinReusable(IReusable* reusable);

	static ReusableCache* sCache;
    static bool sCaching;

    bool mUIModeEnabled;
    uint mType;
    uint mSubType;

	ITileDecorator* mDecorator;
	CCGLProgram* mNormalShader;
	CCGLProgram* mKeyShader;

	GLubyte mSparkleX;
	Twinkle* mTwinkle;
};
#endif // __TILE_DECORATION_H__
