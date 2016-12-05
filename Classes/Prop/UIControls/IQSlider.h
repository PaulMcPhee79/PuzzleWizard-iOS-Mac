#ifndef __IQ_SLIDER_H__
#define __IQ_SLIDER_H__

#include "cocos2d.h"
#include <Prop/Prop.h>
#include <Events/EventDispatcher.h>
USING_NS_CC;

class IQSlider : public Prop, public IEventListener
{
public:
    static int EV_TYPE_IQ_CHANGED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
    
	explicit IQSlider(int min, int max, IEventListener* listener = NULL);
	virtual ~IQSlider(void);
    
	static IQSlider* create(int min, int max, IEventListener* listener = NULL, bool autorelease = true);
	virtual bool init();
    
    CCSize getSliderSize(void) const;
    
    int getIQIntervalValue() const { return mIQValue - (mIQValue % 5); }
    int getIQValue() const { return mIQValue; }
    void setIQValue(int value);
    
    IEventListener* getListener(void) const { return mListener; }
	void setListener(IEventListener* value) { mListener = value; }
    
    virtual void onEvent(int evType, void* evData);
    
protected:
    void setIQValueInternal(int value);
    int sliderPos2IQValue(float value);
    float IQValue2SliderPos(int value);
    int getIQRange() const { return mIQMaxValue - mIQMinValue; }
    float getSliderPosX() const { return mSliderPosX; }
    float getSliderPosRange() const { return mSliderPosMaxX - mSliderPosMinX; }
    void moveSliderBy(float value) { moveSliderTo(getSliderPosX() + value); }
    void moveSliderTo(float value);
    
private:
    bool didTouchSliderKnob(const CCPoint& touchPos);
    void onTouch(int evType, void* evData);
    
    int mIQMinValue;
    int mIQMaxValue;
    int mIQValue;
    float mSliderPosX;
    float mSliderPosMinX;
    float mSliderPosMaxX;
    CCLabelBMFont* mIqLabel;
    CCSprite* mIqTag;
    CCSprite* mIqRule;
    CCNode* mIqTagNode;
    IEventListener* mListener;
};
#endif // __IQ_SLIDER_H__
