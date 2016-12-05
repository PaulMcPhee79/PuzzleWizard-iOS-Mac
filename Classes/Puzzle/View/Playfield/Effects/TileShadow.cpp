
#include "TileShadow.h"
#include <Utils/Utils.h>
#include <Utils/CMTypes.h>
USING_NS_CC;

TileShadow::TileShadow(const CCRect& occRegion)
	:
mOcclusionRegion(occRegion)
{

}

TileShadow::~TileShadow(void)
{
	
}

TileShadow* TileShadow::createWith(CCTexture2D *texture, const CCRect& occRegion, bool autorelease)
{
	TileShadow *tileShadow = new TileShadow(occRegion);
	if (tileShadow && tileShadow->initWithTexture(texture))
    {
		tileShadow->setShaderProgram(CCShaderCache::sharedShaderCache()->programForKey("TileShadow"));
		if (autorelease)
			tileShadow->autorelease();
        return tileShadow;
    }
    CC_SAFE_DELETE(tileShadow);
    return NULL;
}

CCRect TileShadow::getShadowBounds(void)
{
	CCPoint shadowPos = this->getParent()->convertToNodeSpace(this->getParent()->convertToWorldSpace(getPosition()));
	CCRect localBounds = boundingBox();
	CCRect shadowBounds = CCRectMake(
			shadowPos.x - localBounds.size.width / 2,
			shadowPos.y - localBounds.size.height / 2,
			localBounds.size.width,
			localBounds.size.height);
	return shadowBounds;
}

void TileShadow::visit(void)
{
	// Calculate shadow occlusion region in texture coordinates.
	if (this->getParent())
	{
		CCRect shadowBounds = getShadowBounds();
		CCRect occBounds = CMUtils::intersectionRect(shadowBounds, mOcclusionRegion);
		vec4 occRegion = cmv4(
			(occBounds.origin.x - shadowBounds.origin.x) / shadowBounds.size.width,
			(occBounds.origin.y - shadowBounds.origin.y) / shadowBounds.size.height,
			occBounds.size.width,
			occBounds.size.height);
		//occRegion.z = occRegion.x + occRegion.z / shadowBounds.size.width;
		//occRegion.w = occRegion.y + occRegion.w / shadowBounds.size.height;
		occRegion.z = occRegion.z / shadowBounds.size.width;
		occRegion.w = occRegion.w / shadowBounds.size.height;

		CCGLProgram* glProgram = getShaderProgram();
		glProgram->use();
	
		glUniform4f(mScene->uniformLocationByName("u_occRegionTS"), occRegion.x, occRegion.y, MAX(0.001f, occRegion.z), MAX(0.001f, occRegion.w));
		GLuint texId = mScene->textureByName("tile-shadow-occ")->getName();
		ccGLBindTexture2DN(mScene->textureUnitById(texId), texId);
	}

	CMSprite::visit();
}
