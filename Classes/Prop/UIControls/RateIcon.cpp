
#include "RateIcon.h"
#include <Puzzle/Inputs/TouchPad.h>
#include <Utils/Utils.h>
USING_NS_CC;

static const float kContentBaseScale = 1.25f;
static const float kTouchPadding = 40.0f;
static const int kNumStars = 3;
static const int kNumStarPositions = 2 * kNumStars * Localizer::kNumLocales;
// Left-to-right (x,y tuples)
static const float kStarPositions[kNumStarPositions] =
{
    // EN
    -35, 30, 0, 40, 35, 30,
    // CN
    -35, 30, 0, 40, 35, 30,
    // DE
    -35, 30, 0, 40, 35, 30,
    // ES
    -35, 30, 0, 40, 35, 30,
    // FR
    -35, 30, 0, 40, 35, 30,
    // IT
    -35, 30, 0, 40, 35, 30,
    // JP
    -35, 32, 0, 42, 35, 32,
    // KR
    -35, 32, 0, 42, 35, 32
};
static const float kStarScales[kNumStars] =
{
    0.7f, 1.0f, 0.7f
};

RateIcon::RateIcon(int category)
:
Prop(category),
mIsPressed(false),
mLoadingRotation(0),
mText(NULL),
mLoading(NULL),
mContents(NULL),
mContentsScaler(NULL)
{
    mAdvanceable = true;
    mStars.resize(kNumStars, NULL);
}

RateIcon::~RateIcon(void)
{

}

RateIcon* RateIcon::createWithCategory(int category, bool autorelease)
{
	RateIcon* rateIcon = new RateIcon(category);
    if (rateIcon && rateIcon->init())
    {
		if (autorelease)
			rateIcon->autorelease();
        return rateIcon;
    }
    CC_SAFE_DELETE(rateIcon);
    return NULL;
}

bool RateIcon::init(void)
{
    if (mContentsScaler)
		return true;
    
    bool bRet = Prop::init();
    
    if (bRet)
    {
        do
        {
            mContentsScaler = Prop::createWithCategory(-1);
            if (mContentsScaler == NULL) { bRet = false; break; }
            mContentsScaler->setScale(kContentBaseScale);
            addChild(mContentsScaler);
            
            mContents = Prop::createWithCategory(-1);
            if (mContents == NULL) { bRet = false; break; }
            mContentsScaler->addChild(mContents);
            
            for (int i = 0, n = (int)mStars.size(); i < n; ++i)
            {
                CCSprite* star = CCSprite::createWithSpriteFrame(mScene->spriteFrameByName("rate-star"));
                if (star == NULL) { bRet = false; break; }
                star->setScale(kStarScales[i]);
                mStars[i] = star;
                mContents->addChild(star);
            }
            
            if (bRet)
            {
                std::string texName = std::string("rate-text-") + Localizer::locale2StringLower(mScene->getLocale());
                mText = CCSprite::createWithSpriteFrame(mScene->spriteFrameByName(texName.c_str()));
                if (mText == NULL) { bRet = false; break; }
                mContents->addChild(mText);
            }
            
            if (bRet)
            {
                mLoading = CCSprite::createWithTexture(mScene->textureByName("loading"));
                if (mLoading == NULL) { bRet = false; break; }
                mLoading->setVisible(false);
                mContents->addChild(mLoading);
            }
            
            if (bRet)
                layoutContents();
        } while (false);
    }
    
	return bRet;
}

void RateIcon::layoutContents(void)
{
    int index = 2 * kNumStars * (int)mScene->getLocale();
    CCAssert(index < kNumStarPositions, "RateIcon::layoutContents - invalid state.");
    
    if (index < kNumStarPositions)
    {
        mContents->setPosition(ccp(0, -kStarPositions[index+3] / 2)); // y-value of highest star
        
        for (std::vector<CCSprite*>::iterator it = mStars.begin(); it != mStars.end(); ++it)
        {
            (*it)->setPosition(ccp(kStarPositions[index], kStarPositions[index+1]));
            index += 2;
        }
    }
    
    // Update bounds for touch processing
    CCRect cbounds = mText->boundingBox();
    for (std::vector<CCSprite*>::iterator it = mStars.begin(); it != mStars.end(); ++it)
        cbounds = CMUtils::unionRect(cbounds, (*it)->boundingBox());
    
    mLoading->setPosition(ccp(0, cbounds.size.height / 4));
    
    cbounds.size.width *= kContentBaseScale;
    cbounds.size.height *= kContentBaseScale;
	setContentSize(cbounds.size);
}

void RateIcon::animate(bool enable)
{
    if (mLoading)
    {
        mLoading->setVisible(enable);
     
        if (mText)
            mText->setVisible(!enable);
        for (std::vector<CCSprite*>::iterator it = mStars.begin(); it != mStars.end(); ++it)
            (*it)->setVisible(!enable);
    }
}

void RateIcon::advanceTime(float dt)
{
    if (mLoading && mLoading->isVisible())
    {
        mLoadingRotation += 480 * dt;
        
        if (mLoadingRotation >= 360 / 8)
        {
            mLoadingRotation -= 360 / 8;
            mLoading->setRotation(mLoading->getRotation() + 360 / 8);
        }
    }
}

void RateIcon::setVisible(bool visible)
{
    Prop::setVisible(visible);
    reset();
}

void RateIcon::depress(void)
{
    if (mLoading && mLoading->isVisible())
        return;
    
    if (!mIsPressed)
    {
        mContentsScaler->setScale(0.9f * kContentBaseScale);
        mIsPressed = true;
    }
}

void RateIcon::raise(void)
{
    if (mIsPressed)
    {
        mContentsScaler->setScale(kContentBaseScale);
        mIsPressed = false;
        mScene->playSound("button");
        dispatchEvent(EV_TYPE_RATE_ICON_PRESSED(), this);
    }
}

void RateIcon::reset(void)
{
    mContentsScaler->setScale(kContentBaseScale);
    mIsPressed = false;
}

void RateIcon::localeDidChange(const char* fontKey, const char* FXFontKey)
{
    std::string texName = std::string("rate-text-") + Localizer::locale2StringLower(mScene->getLocale());
    mText->setDisplayFrame(mScene->spriteFrameByName(texName.c_str()));
    layoutContents();
}

void RateIcon::onTouch(int evType, void* evData)
{
    if (!isVisible())
        return;

    if (evType != TouchPad::EV_TYPE_TOUCH_BEGAN() && evType != TouchPad::EV_TYPE_TOUCH_MOVED() &&
    		evType != TouchPad::EV_TYPE_TOUCH_ENDED() && evType != TouchPad::EV_TYPE_TOUCH_CANCELLED())
    	return;

    CMTouches::TouchNotice* touchNotice = (CMTouches::TouchNotice*)evData;
	if (touchNotice == NULL)
		return;
    
	CCPoint touchPos = convertToNodeSpace(touchNotice->pos);
    CCSize contentSize = getContentSize();
    CCRect contentBounds = CCRectMake(
                                      -contentSize.width / 2,
                                      -contentSize.height / 2,
                                      contentSize.width,
                                      contentSize.height);
    
	if (evType == TouchPad::EV_TYPE_TOUCH_BEGAN())
	{
        if (!mIsPressed && contentBounds.containsPoint(touchPos))
        {
        	// Note: Leave commented. Focus is retained by our owner which acts as our touch proxy
        	//touchNotice->retainFocus(this);
            depress();
        }
    }
    else if (evType == TouchPad::EV_TYPE_TOUCH_ENDED() || evType == TouchPad::EV_TYPE_TOUCH_CANCELLED())
	{
        raise();
    }
    else if (evType == TouchPad::EV_TYPE_TOUCH_MOVED())
	{
    	if (mIsPressed)
    	{
    		CCRect cancelBounds = contentBounds;
    		cancelBounds.setRect(
                             	 cancelBounds.origin.x - kTouchPadding,
                             	 cancelBounds.origin.y - kTouchPadding,
                             	 cancelBounds.size.width + 2 * kTouchPadding,
                             	 cancelBounds.size.height + 2 * kTouchPadding);
    		if (!cancelBounds.containsPoint(touchPos))
    			reset();
    	}
    }
}
