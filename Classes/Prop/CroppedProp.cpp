
#include "CroppedProp.h"
#include "AppMacros.h"
#include <Utils/Utils.h>
USING_NS_CC;

CroppedProp::CroppedProp(int category, const CCRect& viewableRegion)
	: Prop(category)
    , mEnabled(true)
    , mViewableRegion(viewableRegion)
{
	
}

CroppedProp::~CroppedProp(void)
{
}

CCRect CroppedProp::getViewableRegion()
{
	return mViewableRegion;
}

void CroppedProp::setViewableRegion(const CCRect& value)
{
	mViewableRegion = value;
}

void CroppedProp::verifyViewableRegion()
{
	CCPoint origin = CCDirector::sharedDirector()->getVisibleOrigin();
	CCSize size =  CCDirector::sharedDirector()->getVisibleSize();
	CCRect viewBounds = CCRectMake(origin.x, origin.y, size.width, size.height);
	this->setViewableRegion(CMUtils::intersectionRect(viewBounds, mViewableRegion));
}

void CroppedProp::clampToContent()
{
	CCRect bounds = this->boundingBox();
	CCPoint topLeft = convertToWorldSpace(CCPointZero);
	CCPoint btmRight = convertToWorldSpace(CCPointMake(bounds.size.width, bounds.size.height));
	this->setViewableRegion(CCRectMake(topLeft.x, topLeft.y, btmRight.x, btmRight.y));
}

void CroppedProp::visit(void)
{
    this->verifyViewableRegion();
    
    if (mEnabled)
    {
        kmGLPushMatrix();

        glEnable(GL_SCISSOR_TEST);
        
        CCEGLView::sharedOpenGLView()->setScissorInPoints(
            mViewableRegion.getMinX(),
            mViewableRegion.getMinY(),
            mViewableRegion.size.width,
            mViewableRegion.size.height);

        Prop::visit();
        glDisable(GL_SCISSOR_TEST);
        kmGLPopMatrix();
    }
    else
        Prop::visit();
}
