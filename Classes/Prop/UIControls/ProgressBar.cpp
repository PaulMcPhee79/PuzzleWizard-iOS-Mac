
#include "ProgressBar.h"
#include <Utils/Utils.h>
USING_NS_CC;

ProgressBar::ProgressBar(int category)
:
Prop(category),
mProgressPercentage(0),
mTrackBar(NULL),
mProgressBar(NULL)
{

}

ProgressBar::~ProgressBar(void)
{

}

ProgressBar* ProgressBar::create(int category, float width, float height, bool autorelease)
{
    
	ProgressBar *progressBar = new ProgressBar(category);
    if (progressBar && progressBar->initWith(width, height))
    {
		if (autorelease)
			progressBar->autorelease();
        return progressBar;
    }
    CC_SAFE_DELETE(progressBar);
    return NULL;
}

bool ProgressBar::initWith(float width, float height)
{
    if (mTrackBar)
		return true;
    
    bool bRet = Prop::init();
    
    if (bRet)
    {
        do
        {
            mTrackBar = CMUtils::createColoredQuad(CCSizeMake(width, height));
            if (mTrackBar == NULL) { bRet = false; break; }
            mTrackBar->setColor(CMUtils::uint2color3B(0xffffff));
            addChild(mTrackBar);
            
            mProgressBar = CMUtils::createColoredQuad(CCSizeMake(width, height));
            if (mProgressBar == NULL) { bRet = false; break; }
            mProgressBar->setPositionX(-width / 2);
            mProgressBar->setAnchorPoint(ccp(0, 0.5f));
            mProgressBar->setColor(CMUtils::uint2color3B(0x00ff00));
            mProgressBar->setVisible(mProgressPercentage != 0);
            addChild(mProgressBar);
        } while (false);
    }
    
	return bRet;
}

void ProgressBar::setTrackColor(uint color)
{
    mTrackBar->setColor(CMUtils::uint2color3B(color));
}

void ProgressBar::setProgressColor(uint color)
{
    mProgressBar->setColor(CMUtils::uint2color3B(color));
}

void ProgressBar::reconfigure(float width, float height)
{
    mTrackBar->setTextureRect(CCRectMake(0, 0, width, height));
    mProgressBar->setTextureRect(CCRectMake(0, 0, width, height));
    mProgressBar->setPositionX(-width / 2);
}

void ProgressBar::setProgress(float value)
{
    if (value != mProgressPercentage)
    {
        mProgressPercentage = value;
        
        CCRect trackTexRect = mTrackBar->getTextureRect(), progTexRect = mProgressBar->getTextureRect();
        mProgressBar->setTextureRect(CCRectMake(0, 0, MAX(1, value * trackTexRect.size.width), progTexRect.size.height));
        mProgressBar->setVisible(value != 0);
    }
}
