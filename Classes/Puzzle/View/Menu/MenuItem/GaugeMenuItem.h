#ifndef __GAUGE_MENU_ITEM_H__
#define __GAUGE_MENU_ITEM_H__

#include "cocos2d.h"
#include "MenuItem.h"
USING_NS_CC;

typedef struct GaugeMenuStruct
{
    GaugeMenuStruct(void)
    :
      listener(NULL)
    , upFrame(NULL)
    , gaugeOffset(80)
    , fontSize(52)
    , text(NULL)
    { }
    
    IEventListener* listener;
    CCSpriteFrame* upFrame;
    float gaugeOffset;
    int fontSize;
    const char* text;
} GaugeMenuStruct;

class GaugeMenuItem : public MenuItem
{
public:
    static int EV_TYPE_GAUGE_CHANGED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
    
	GaugeMenuItem(void);
	virtual ~GaugeMenuItem(void);
    
	static GaugeMenuItem* create(const GaugeMenuStruct& initData, bool autorelease = true);
    virtual bool initWith(const GaugeMenuStruct& initData);
    
    virtual void localeDidChange(const char* fontKey, const char* FXFontKey);
    
    bool shouldPlaySound(void) const { return mShouldPlaySound; }
    void setShouldPlaySound(bool value) { mShouldPlaySound = value; }
    int getGaugeLevel(void) const { return mGaugeLevel; }
    void setGaugeLevel(int value);
    uint getStrokeFilledColor(void) const { return mStrokeFilledColor; }
    void setStrokeFilledColor(uint value);
    uint getStrokeEmptyColor(void) const { return mStrokeEmptyColor; }
    void setStrokeEmptyColor(uint value);
    
    virtual void setSelected(bool value)
    {
        if (value != isSelected())
            resetInertia();
        
        MenuItem::setSelected(value);
    }
    virtual void depress(void) { }
	virtual void raise(void) { }
    virtual void update(int controllerState);

protected:
    virtual void updateContentColor(void);
    virtual void updateContentBounds(void);
    virtual void repositionActiveIcon(void);
    
private:
    void refreshGaugeDisplay(void);
    void layoutCanvasContents(void);
    void resetInertia(void);
    void addToGaugeLevel(int value);
    
    bool mShouldPlaySound;
    int mInputInertia;
    int mInputThreshold;
    int mGaugeLevel;
    float mGaugeOffset;
    uint mStrokeFilledColor;
    uint mStrokeEmptyColor;
    CCLabelBMFont* mLabel;
    CCLabelBMFont* mFillStrokes;
    CCLabelBMFont* mEmptyStrokes;
    CCNode* mCanvas;
};
#endif // __GAUGE_MENU_ITEM_H__
