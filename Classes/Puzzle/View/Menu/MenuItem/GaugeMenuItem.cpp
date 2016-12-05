
#include "GaugeMenuItem.h"
#include <Puzzle/View/Menu/MenuBuilder.h>
#include <Managers/InputManager.h>
#include <Utils/Utils.h>
#include <Utils/LangFX.h>
USING_NS_CC;

const int kNumStrokes = 10;
//const float kLabelSeparation = 12.0f;
const float kGaugeSeparation = 0.0f;
const char kStrokeChar = 'l';
const char* kFullStrokeString = "llllllllll";
const char* kHalfStrokeString = "lllll";

GaugeMenuItem::GaugeMenuItem(void)
:
mShouldPlaySound(true),
mInputInertia(0),
mInputThreshold(30),
mGaugeLevel(kNumStrokes / 2),
mGaugeOffset(20),
mStrokeFilledColor(MenuBuilder::kMenuOrange),
mStrokeEmptyColor(MenuBuilder::kMenuBuilderDisabled),
mLabel(NULL),
mFillStrokes(NULL),
mEmptyStrokes(NULL),
mCanvas(NULL)
{

}

GaugeMenuItem::~GaugeMenuItem(void)
{
    CC_SAFE_RELEASE_NULL(mCanvas);
}

GaugeMenuItem* GaugeMenuItem::create(const GaugeMenuStruct& initData, bool autorelease)
{
    GaugeMenuItem *item = new GaugeMenuItem();
    if (item && item->initWith(initData))
    {
        item->repositionActiveIcon();
		if (autorelease)
			item->autorelease();
        return item;
    }
    CC_SAFE_DELETE(item);
    return NULL;
}

bool GaugeMenuItem::initWith(const GaugeMenuStruct& initData)
{
    bool bRet = MenuItem::initWith(initData.upFrame, initData.upFrame, initData.listener);
    
    if (bRet)
    {
        mGaugeOffset = initData.gaugeOffset;
        
        mLabel = TextUtils::create(initData.text, initData.fontSize, kCCLabelAutomaticWidth, kCCTextAlignmentRight, CCPointZero);
        mLabel->setAnchorPoint(ccp(1.0f, 0.5f));
        mLabel->setColor(CMUtils::uint2color3B(getLabelColorForIndex(kEnabledIndex)));
        
        mFillStrokes = TextUtils::createCommonLGE(kFullStrokeString, initData.fontSize, kCCLabelAutomaticWidth, kCCTextAlignmentLeft, CCPointZero);
        mFillStrokes->setTag(ILocalizable::kNonLocalizableTag);
        mFillStrokes->setAnchorPoint(CCPointZero);
        mFillStrokes->setScale(0.9f * mFillStrokes->getScale());
        mFillStrokes->setPositionY(mLabel->getPositionY() + LangFX::getGaugeStrokeYOffset()
                                   + (mLabel->boundingBox().size.height - mFillStrokes->boundingBox().size.height) / 2
                                   - mLabel->boundingBox().size.height / 2);
        mFillStrokes->setColor(CMUtils::uint2color3B(mStrokeFilledColor));
        
        mEmptyStrokes = TextUtils::createCommonLGE(kFullStrokeString, initData.fontSize, kCCLabelAutomaticWidth, kCCTextAlignmentLeft, CCPointZero);
        mEmptyStrokes->setTag(ILocalizable::kNonLocalizableTag);
        mEmptyStrokes->setAnchorPoint(CCPointZero);
        mEmptyStrokes->setScale(mFillStrokes->getScale());
        mEmptyStrokes->setPositionY(mFillStrokes->getPositionY());
        mEmptyStrokes->setColor(CMUtils::uint2color3B(mStrokeEmptyColor));
        
        mCanvas = new CCNode();
        mCanvas->addChild(mLabel);
        mCanvas->addChild(mFillStrokes);
        mCanvas->addChild(mEmptyStrokes);
        mContents->addChild(mCanvas);
        
        mFillStrokes->setString(kHalfStrokeString);
        mEmptyStrokes->setString(kHalfStrokeString);
        
        setScaleWhenPressed(1.0f);
        
        layoutCanvasContents();
        updateContentBounds();
        repositionActiveIcon();
    }
    
    return bRet;
}

void GaugeMenuItem::localeDidChange(const char* fontKey, const char* FXFontKey)
{
    TextUtils::switchFntFile(fontKey, mLabel);
    
    if (mFillStrokes && mEmptyStrokes)
    {
        mFillStrokes->setPositionY(mLabel->getPositionY() + LangFX::getGaugeStrokeYOffset()
                                   + (mLabel->boundingBox().size.height - mFillStrokes->boundingBox().size.height) / 2
                                   - mLabel->boundingBox().size.height / 2);
        mEmptyStrokes->setPositionY(mFillStrokes->getPositionY());
    }
    
    MenuItem::localeDidChange(fontKey, FXFontKey);
}

void GaugeMenuItem::setGaugeLevel(int value)
{
    int val = MAX(0, MIN(kNumStrokes, value));
    if (val != mGaugeLevel)
    {
        mGaugeLevel = val;
        refreshGaugeDisplay();
        layoutCanvasContents();
    }
}

void GaugeMenuItem::setStrokeFilledColor(uint value)
{
    mStrokeFilledColor = value;
    
    if (!isSelected() && mFillStrokes)
        mFillStrokes->setColor(CMUtils::uint2color3B(value));
}

void GaugeMenuItem::setStrokeEmptyColor(uint value)
{
    mStrokeEmptyColor = value;
    
    if (!isSelected() && mEmptyStrokes)
        mEmptyStrokes->setColor(CMUtils::uint2color3B(value));
}

void GaugeMenuItem::updateContentColor(void)
{
    MenuItem::updateContentColor();
    
    if (mLabel)
        mLabel->setColor(isSelected()
                         ? CMUtils::uint2color3B(getLabelColorForIndex(MenuButton::kSelectedIndex))
                         : CMUtils::uint2color3B(getLabelColorForIndex(MenuButton::kEnabledIndex)));
    if (mFillStrokes)
        mFillStrokes->setColor(isSelected()
                               ? CMUtils::uint2color3B(getLabelColorForIndex(MenuButton::kSelectedIndex))
                               : CMUtils::uint2color3B(getStrokeFilledColor()));
}

void GaugeMenuItem::updateContentBounds(void)
{
	MenuItem::updateContentBounds();
    
    if (mLabel && mFillStrokes && mEmptyStrokes)
    {
        CCRect gaugeRect = CMUtils::unionRect(
                                              CMUtils::unionRect(mLabel->boundingBox(), mFillStrokes->boundingBox()),
                                              mEmptyStrokes->boundingBox());
        CCRect cbounds = CMUtils::unionRect(gaugeRect, CCRectMake(0, 0, getContentSize().width, getContentSize().height));
        setContentSize(CCSizeMake(cbounds.size.width, cbounds.size.height));
    }
}

void GaugeMenuItem::refreshGaugeDisplay(void)
{
    if (mFillStrokes)
        mFillStrokes->setString(std::string(mGaugeLevel, kStrokeChar).c_str());
    if (mEmptyStrokes)
        mEmptyStrokes->setString(std::string(kNumStrokes - mGaugeLevel, kStrokeChar).c_str());
}

void GaugeMenuItem::layoutCanvasContents(void)
{
    mLabel->setPositionX(0);
    mFillStrokes->setPositionX(mLabel->boundingBox().getMaxX() + mGaugeOffset);
    mEmptyStrokes->setPositionX(mFillStrokes->getPositionX() + (mGaugeLevel > 0 ? mFillStrokes->boundingBox().size.width : 0) + kGaugeSeparation);
}

void GaugeMenuItem::resetInertia(void)
{
    mInputInertia = 0;
    mInputThreshold = (int)(0.35f * (1.0 / (CCDirector::sharedDirector()->getAnimationInterval())));
}

void GaugeMenuItem::repositionActiveIcon(void)
{
    if (mSelectedIcon && mLabel)
    {
        mSelectedIcon->removeFromParent();
        mSelectedIcon->setPosition(ccp(
                                       mLabel->boundingBox().getMinX() - (mSelectedIcon->boundingBox().size.width + 36),
                                       mLabel->getPositionY() + LangFX::getActiveIconYOffset()));
        addBgContent(mSelectedIcon);
    }
}

void GaugeMenuItem::addToGaugeLevel(int value)
{
    int prevValue = getGaugeLevel();
    setGaugeLevel(getGaugeLevel() + value);
    if (prevValue != getGaugeLevel() && getListener())
        getListener()->onEvent(EV_TYPE_GAUGE_CHANGED(), this);
}

void GaugeMenuItem::update(int controllerState)
{
    if (!isSelected())
        return;
    
    InputManager* im = InputManager::IM();
    Coord depressedVector = im->getDepressedVector(), heldVector = im->getHeldVector();
    if (depressedVector.x != 0)
    {
        addToGaugeLevel(depressedVector.x);
        resetInertia();
    }
    else if (heldVector.x != 0)
    {
        if (++mInputInertia >= mInputThreshold)
        {
            mInputInertia = 0;
            mInputThreshold = (int)MAX(0.1f * (1.0 / (CCDirector::sharedDirector()->getAnimationInterval())),  0.75f * mInputThreshold);
            addToGaugeLevel(heldVector.x);
        }
    }
    else
        resetInertia();
}

