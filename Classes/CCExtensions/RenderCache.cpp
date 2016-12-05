
#include "RenderCache.h"
#include "RenderNode.h"
#include <Utils/Utils.h>
USING_NS_CC;

static const int kBorderPadding = 0; //14;
static const int kNodePadding = 0; //14;

RenderCache::RenderCache(int w, int h)
    : mDirty(false)
    , mWidth(w)
    , mHeight(h)
    , mNodes(NULL)
    , mNodesDict(NULL)
{
    
}

RenderCache::~RenderCache(void)
{
    CCSpriteFrameCache::sharedSpriteFrameCache()->removeSpriteFramesFromTexture(m_pTexture);
	CC_SAFE_RELEASE_NULL(mNodes);
    CC_SAFE_RELEASE_NULL(mNodesDict);
}

RenderCache* RenderCache::create(int w, int h, CCTexture2DPixelFormat eFormat, bool autorelease)
{
    RenderCache *pRet = new RenderCache(w, h);
    
    if(pRet && pRet->init(w, h, eFormat))
    {
        if (autorelease)
            pRet->autorelease();
        return pRet;
    }
    CC_SAFE_DELETE(pRet);
    return NULL;
}

bool RenderCache::init(int w, int h, CCTexture2DPixelFormat eFormat)
{
    mNodes = CCArray::create();
    if (mNodes)
        mNodes->retain();
    
    mNodesDict = CCDictionary::create();
    if (mNodesDict)
        mNodesDict->retain();
    
    return mNodes && mNodesDict && CCRenderTexture::initWithWidthAndHeight(w, h, eFormat);
}

int RenderCache::getBorderPadding(void)
{
    return kBorderPadding;
}

void RenderCache::addCacheNode(CCNode* node, const char* name)
{
    CCAssert(node, "RenderCache::addCacheNode - invalid args.");
    addCacheNode(node, name, node->boundingBox());
}

void RenderCache::addCacheNode(CCNode* node, const char* name, const CCRect& frame)
{
    CCAssert(node && name, "RenderCache::addCacheNode - invalid args.");
    CCAssert(mNodesDict->objectForKey(name) == NULL, "RenderCache::addCacheNode - duplicate keys not permitted.");
    
    CCPoint nodeOrigin = nextFreeNodePosition();
    CCAssert(nodeOrigin.x + node->boundingBox().size.width <= mWidth && nodeOrigin.y + node->boundingBox().size.height <= mHeight,
             "RenderCache::addCacheNode - node does not fit.");
    
//    CCRect renderFrame = CCRectMake(nodeOrigin.x + frame.size.width / 2, nodeOrigin.y + frame.size.height / 2, frame.size.width, frame.size.height);
    CCRect renderFrame = CCRectMake(nodeOrigin.x, nodeOrigin.y, frame.size.width, frame.size.height);
    RenderNode* renderNode = RenderNode::create(node, renderFrame);

    mNodes->addObject(renderNode);
    mNodesDict->setObject(renderNode, name);
    mDirty = true;
}

void RenderCache::removeCacheNode(const char* name)
{
    if (name)
    {
        CCObject* obj = mNodesDict->objectForKey(name);
        if (obj)
        {
            mNodesDict->removeObjectForKey(name);
            mNodes->removeObject(obj);
            mDirty = true;
        }
    }
}

void RenderCache::clearCache(void)
{
    if (mNodes->count() != 0)
    {
        mNodes->removeAllObjects();
        mNodesDict->removeAllObjects();
        mDirty = true;
    }
}

CCSpriteFrame* RenderCache::spriteFrameByName(const char *name)
{
    if (name)
        return CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(name);
    else
        return NULL;
}

// Build atlas from bottom to top.
CCPoint RenderCache::nextFreeNodePosition(void)
{
    CCPoint pt(kBorderPadding, kBorderPadding);
    unsigned int count = mNodes->count();
    if (count > 0)
    {
        CCObject* obj = mNodes->objectAtIndex(count - 1);
        if (obj)
        {
            RenderNode* renderNode = static_cast<RenderNode*>(obj);
            pt.y = renderNode->mFrame.getMaxY() + kNodePadding;
        }
    }
    
    return pt;
}

CCRect RenderCache::node2SpriteFrame(const CCRect& rect)
{
    CCRect convertedRect = CCRectMake(rect.origin.x - rect.size.width / 2,
                                      rect.origin.y - rect.size.height / 2,
                                      rect.size.width,
                                      rect.size.height);
    return convertedRect;
}

CCDictionary* RenderCache::generateSpriteFrames(void)
{
    CCDictionary* spriteFrames = CCDictionary::create();
    CCDictElement* kvp;
    CCDICT_FOREACH(mNodesDict, kvp)
    {
        RenderNode* renderNode = static_cast<RenderNode*>(kvp->getObject());
        if (renderNode)
		{
			CCSpriteFrame* frame = CCSpriteFrame::createWithTexture(m_pTexture, renderNode->mFrame);
            spriteFrames->setObject(frame, kvp->getStrKey());
		}
    }
    
    return spriteFrames;
}

void RenderCache::regenerateSpriteFrameCache(void)
{
    CCSpriteFrameCache::sharedSpriteFrameCache()->removeSpriteFramesFromTexture(m_pTexture);
    CCDictionary* spriteFrames = generateSpriteFrames();
    
    if (spriteFrames)
    {
        CCDictElement* kvp;
        CCDICT_FOREACH(spriteFrames, kvp)
        {
            CCSpriteFrame* frame = static_cast<CCSpriteFrame*>(kvp->getObject());
            if (frame)
                CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFrame(frame, kvp->getStrKey());
        }
    }
    
    mDirty = false;
}

void RenderCache::renderCache(void)
{
    ccTexParams params = {GL_LINEAR,GL_LINEAR,GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE};
    m_pTexture->setTexParameters(&params);
    
    if (mDirty)
        regenerateSpriteFrameCache();
 
    beginWithClear(0, 0, 0, 0);
    
    CCObject* obj;
	CCARRAY_FOREACH(mNodes, obj)
	{
		RenderNode* renderNode = static_cast<RenderNode*>(obj);
		if (renderNode)
        {
            CCNode* node = renderNode->mNode;
            CCPoint prevPos = node->getPosition();
            node->setPosition(ccp(renderNode->mFrame.origin.x + renderNode->mFrame.size.width / 2 +
                                  (renderNode->mFrame.size.width - node->boundingBox().size.width) / 2,
                                  renderNode->mFrame.origin.y + renderNode->mFrame.size.height / 2 +
                                  (renderNode->mFrame.size.height - node->boundingBox().size.height) / 2));
//            node->setPosition(renderNode->mFrame.origin);
            node->visit();
            node->setPosition(prevPos);
        }
	}
    
    end();
}
