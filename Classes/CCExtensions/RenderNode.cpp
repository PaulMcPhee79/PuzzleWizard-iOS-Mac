
#include "RenderNode.h"
USING_NS_CC;

RenderNode::RenderNode(CCNode* node, const CCRect& frame)
    : mFrame(frame)
    , mNode(node)
{

}

RenderNode::~RenderNode(void)
{
    mNode = NULL;
}

RenderNode* RenderNode::create(CCNode* node, const CCRect& frame, bool autorelease)
{
    RenderNode *pRet = new RenderNode(node, frame);
    
    if(pRet)
    {
        if (autorelease)
            pRet->autorelease();
        return pRet;
    }
    CC_SAFE_DELETE(pRet);
    return NULL;
}
