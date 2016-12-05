#ifndef __TILE_SHADOW_H__
#define __TILE_SHADOW_H__

#include "cocos2d.h"
#include <Prop/CMSprite.h>
USING_NS_CC;

class TileShadow : public CMSprite
{
public:
	TileShadow(const CCRect& occRegion);
	virtual ~TileShadow(void);

	static TileShadow* createWith(CCTexture2D *texture, const CCRect& occRegion, bool autorelease = true);

	virtual void visit(void);

	const CCRect& getOcclusionRegion(void) const { return mOcclusionRegion; }
	void setOcclusionRegion(const CCRect& value) { mOcclusionRegion = value; }
	CCRect getShadowBounds(void);

private:
    CCRect mOcclusionRegion;
};
#endif // __TILE_SHADOW_H__
