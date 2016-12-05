
#include "CMSprite.h"
#include <Interfaces/IReusable.h>
#include <Utils/Utils.h>
USING_NS_CC;

SceneController* CMSprite::s_Scene = NULL;

CMSprite::CMSprite(void)
:
mScene(s_Scene),
mAdvanceable(false)
{

}

CMSprite::~CMSprite(void)
{
	mScene = NULL;
}

CMSprite* CMSprite::create(bool autorelease)
{
    CMSprite *sprite = new CMSprite();
	if (sprite && sprite->init())
    {
		if (autorelease)
			sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return NULL;
}

CMSprite* CMSprite::createWithSpriteFrame(CCSpriteFrame* pSpriteFrame, bool autorelease)
{
	CMSprite *sprite = new CMSprite();
    if (pSpriteFrame && sprite && sprite->initWithSpriteFrame(pSpriteFrame))
    {
		if (autorelease)
			sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return NULL;
}

CMSprite* CMSprite::createWithTexture(CCTexture2D *pTexture, bool autorelease)
{
    CMSprite *sprite = new CMSprite();
    if (sprite && sprite->initWithTexture(pTexture))
    {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return NULL;
}

void CMSprite::setOpacityChildren(GLubyte value)
{
	setOpacity(value);
	CMUtils::setOpacity(this, value);
}

void CMSprite::setTextureCoordsCM(CCRect rect)
{
    CCSprite::setTextureCoords(rect);
}

void CMSprite::advanceTime(float dt) { }

SceneController* CMSprite::getCMSpriteScene()
{
	return s_Scene;
}

void CMSprite::setCMSpriteScene(SceneController* value)
{
	if (value)
		s_Scene = value;
}

void CMSprite::relinquishCMSpriteScene(SceneController* scene)
{
	if (scene && scene == s_Scene)
		s_Scene = NULL;
}
