#ifndef __RENDER_CACHE_H__
#define __RENDER_CACHE_H__

#include "cocos2d.h"
USING_NS_CC;

class RenderCache : public CCRenderTexture
{
public:
	RenderCache(int w, int h);
	virtual ~RenderCache(void);

	static RenderCache* create(int w, int h, CCTexture2DPixelFormat eFormat = kCCTexture2DPixelFormat_RGBA8888, bool autorelease = true);
	bool init(int w, int h, CCTexture2DPixelFormat eFormat);
    
    static int getBorderPadding(void);
    CCSpriteFrame* spriteFrameByName(const char *name);
    
    void addCacheNode(CCNode* node, const char* name);
    void addCacheNode(CCNode* node, const char* name, const CCRect& frame);
    void removeCacheNode(const char* name);
    void clearCache(void);
    
    void renderCache(void);
    
protected:
    CCRect node2SpriteFrame(const CCRect& rect);
    CCPoint nextFreeNodePosition(void);
    CCDictionary* generateSpriteFrames(void);
    void regenerateSpriteFrameCache(void);
    
private:
    bool mDirty;
    int mWidth, mHeight;
    CCArray* mNodes;
    CCDictionary* mNodesDict;
};
#endif // __RENDER_CACHE_H__
