#ifndef __SPRITE_LAYER_MANAGER_H__
#define __SPRITE_LAYER_MANAGER_H__

#include "cocos2d.h"
USING_NS_CC;

class SpriteLayerManager
{
public:
	SpriteLayerManager(CCNode* baseLayer, int layerCount);
	virtual ~SpriteLayerManager(void);

	void addChild(CCNode* child, int category);
	void removeChild(CCNode* child, int category, bool cleanup = true);
	CCNode* childAtCategory(int category);
	void clearAllLayers(void);
	void clearAll(void);

protected:
	CCNode* mBase;
};
#endif // __SPRITE_LAYER_MANAGER_H__
