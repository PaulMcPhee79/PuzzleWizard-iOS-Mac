#ifndef __RENDER_NODE_H__
#define __RENDER_NODE_H__

#include "cocos2d.h"
USING_NS_CC;

class RenderNode : public CCObject
{
public:
    RenderNode(CCNode* node, const CCRect& frame);
    virtual ~RenderNode(void);
    
    static RenderNode* create(CCNode* node, const CCRect& frame, bool autorelease = true);
    
    CCRect mFrame;
    CCNode* mNode;
};

#endif // __RENDER_NODE_H__
