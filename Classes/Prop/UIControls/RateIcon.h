#ifndef __RATE_ICON_H__
#define __RATE_ICON_H__

#include "cocos2d.h"
#include <Prop/Prop.h>
#include <Events/EventDispatcher.h>
#include <Interfaces/ILocalizable.h>
USING_NS_CC;

class RateIcon : public Prop, public EventDispatcher, public ILocalizable
{
public:
    static int EV_TYPE_RATE_ICON_PRESSED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
    
	RateIcon(int category);
	virtual ~RateIcon(void);
    
	static RateIcon* createWithCategory(int category, bool autorelease = true);
	virtual bool init(void);
    
    virtual void setVisible(bool visible);
    bool isPressed(void) const { return mIsPressed; }
    void reset(void);
    
    void animate(bool enable);
    virtual void advanceTime(float dt);
    
    virtual void localeDidChange(const char* fontKey, const char* FXFontKey);
    void onTouch(int evType, void* evData);
    
private:
    void layoutContents(void);
    void depress(void);
    void raise(void);

    bool mIsPressed;
    float mLoadingRotation;
    CCSprite* mText;
    CCSprite* mLoading;
    std::vector<CCSprite*> mStars;
    Prop* mContents;
    Prop* mContentsScaler;
};
#endif // __RATE_ICON_H__
