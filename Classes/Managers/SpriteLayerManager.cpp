
#include "SpriteLayerManager.h"
#include "ccMacros.h"

SpriteLayerManager::SpriteLayerManager(cocos2d::CCNode* baseLayer, int layerCount)
{
	CCAssert(baseLayer, "SpriteLayerManager: baseLayer cannot be null.");
	mBase = baseLayer;
	int count = MAX(1, layerCount);

	for (int i = 0; i < count; ++i)
	{
		CCNode *node = CCNode::create();
		node->setTag(i);
		mBase->addChild(node);
	}
}

SpriteLayerManager::~SpriteLayerManager(void)
{
	// Do nothing
}

void SpriteLayerManager::addChild(CCNode* child, int category)
{
	if (category < (int)mBase->getChildrenCount())
		childAtCategory(category)->addChild(child);
}

void SpriteLayerManager::removeChild(CCNode* child, int category, bool cleanup)
{
	childAtCategory(category)->removeChild(child, cleanup);
}

CCNode* SpriteLayerManager::childAtCategory(int category)
{
	return mBase->getChildByTag(category);
}

void SpriteLayerManager::clearAllLayers(void)
{
	CCObject* child;
	CCARRAY_FOREACH(mBase->getChildren(), child)
    {
        CCNode* node = static_cast<CCNode*>(child);
        if(node)
			node->removeAllChildren();
    }
}

void SpriteLayerManager::clearAll(void)
{
	mBase->removeAllChildren();
}
