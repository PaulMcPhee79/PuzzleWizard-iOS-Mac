#ifndef __MENU_BUTTON_H__
#define __MENU_BUTTON_H__

#include "cocos2d.h"
#include <Prop/Prop.h>
#include <Events/EventDispatcher.h>
#include <Interfaces/ILocalizable.h>
USING_NS_CC;

class MenuButton : public Prop, public IEventListener, public ILocalizable
{
public:
	static int EV_TYPE_PRESSED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
	static int EV_TYPE_RAISED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
    static int EV_TYPE_SELECTED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
    static int EV_TYPE_UNSELECTED() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }
    static int EV_TYPE_RESET() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }

	MenuButton(void);
	MenuButton(int fontSize);
	virtual ~MenuButton(void);

	static MenuButton* create(bool autorelease = true);
	static MenuButton* create(CCSpriteFrame* upFrame, CCSpriteFrame* downFrame = NULL, IEventListener* listener = NULL, bool autorelease = true);
	static MenuButton* create(const char* text, int fontSize = 32, IEventListener* listener = NULL, bool autorelease = true);

	virtual bool initWith(CCSpriteFrame* upFrame, CCSpriteFrame* downFrame = NULL, IEventListener* listener = NULL);

    virtual void localeDidChange(const char* fontKey, const char* FXFontKey);
    
	bool isEnabled(void) const { return getState() != MenuButton::DISABLED; }
    bool isNavigable(void) const { return true; }
	bool isPressed(void) const { return getState() == MenuButton::PRESSED; }
	bool isReleased(void) const { return getState() == MenuButton::ENABLED; }
    bool isSelected(void) const { return mSelected; }
    virtual void setSelected(bool value)
    {
        if (mSelected != value)
        {
            mSelected = value;
            updateDisplayFrame();
            updateContentColor();
            
            if (mListener)
                mListener->onEvent(value ? EV_TYPE_SELECTED() : EV_TYPE_UNSELECTED(), this);
        }
    }
    void setSelectedDisplayFrame(CCSpriteFrame* value);

	const char* getString(void) { return mLabel ? mLabel->getString() : NULL; }
	void setString(const char* value);
	void setFontSize(int value);
	// kCCTextAlignmentLeft, kCCTextAlignmentCenter, kCCTextAlignmentRight
	void setAlignmentH(CCTextAlignment alignment);
	// kCCVerticalTextAlignmentTop, kCCVerticalTextAlignmentCenter, kCCVerticalTextAlignmentBottom
	void setAlignmentV(CCVerticalTextAlignment alignment);
    void setFontAnchorPoint(const CCPoint& pt);
	void setEnabledOpacity(GLubyte value);
	void setEnabledLabelColor(uint value);
	void setDisabledLabelColor(uint value);
	void setPressedLabelColor(uint value);
    void setSelectedLabelColor(uint value);
    
    void setContentColor(uint value);
    void setEnabledFrameColor(uint value);
	void setDisabledFrameColor(uint value);
	void setPressedFrameColor(uint value);
    void setSelectedFrameColor(uint value);

	virtual void enable(bool enable);
	virtual void depress(void);
	virtual void raise(void);
	virtual void reset(void);
	void enforceOpacity(GLubyte opacity);

	void addBgContent(CCNode* content);
	void removeBgContent(CCNode* content);
    void addBgContent(CCNode* content, CCRGBAProtocol* colorable);
	void removeBgContent(CCNode* content, CCRGBAProtocol* colorable);

    void setMinTouchSize(const CCSize& touchSize);
	virtual void enableTouch(bool enable, CMTouches::TouchCategory category);
	virtual void onEvent(int evType, void* evData);
	IEventListener* getListener(void) const { return mListener; }
	void setListener(IEventListener* value) { mListener = value; }

protected:
	enum MenuButtonState { ENABLED = 0, DISABLED, PRESSED };

    static const int kEnabledIndex = 0;
    static const int kDisabledIndex = 1;
    static const int kPressedIndex = 2;
    static const int kSelectedIndex = 3;
    
	MenuButtonState getState(void) const { return mState; }
	virtual void setState(MenuButtonState value);
    float getDisabledOpacityFactor(void) const { return mDisabledOpacityFactor; }
    void setDisabledOpacityFactor(float value) { mDisabledOpacityFactor = value; }
    float getScaleWhenPressed(void) const { return mScaleWhenPressed; }
    void setScaleWhenPressed(float value) { mScaleWhenPressed = value; }
    uint getFrameColorForIndex(int index)
    {
        if (index == kSelectedIndex)
            return mSelectedColor;
        else if (index >= kEnabledIndex && index <= kPressedIndex)
            return mFrameColors[index];
        else
            return 0xffffff;
    }
    uint getLabelColorForIndex(int index)
    {
        if (index == kSelectedIndex)
            return mSelectedColor;
        else if (index >= kEnabledIndex && index <= kPressedIndex)
            return mLabelColors[index];
        else
            return 0xffffff;
    }
    virtual void updateContentColor(void);
    virtual void updateContentBounds(void);

private:
	static const int kNumDisplayFrames = 4;
    
    int getIndexForState(MenuButtonState state);
	CCLabelBMFont* createLabel(const char* text);
    void updateDisplayFrame(void);
	void updateAlignment(void);
    void setUpScaleChildren(void);
    void setDownScaleChildren(void);

    bool mSelected;
    uint mSelectedColor;
    uint mSelectedLabelColor;
    float mDisabledOpacityFactor;
    float mScaleWhenPressed;
    
    float mChildScales[3];
	MenuButtonState mState;
	GLubyte mEnabledOpacity;

    uint mFrameColors[3];
    CCRect mTouchBounds;
	CCSprite* mSprite;
	CCSpriteFrame* mSpriteFrames[kNumDisplayFrames];
	CCNode* mBgNode;

	int mLabelFontSize;
	uint mLabelColors[3];
	CCTextAlignment mLabelAlignH;
	CCVerticalTextAlignment mLabelAlignV;
	CCLabelBMFont* mLabel;
    
    std::vector<CCRGBAProtocol*>* mColorables;

	CMTouches::TouchCategory mTouchCategory;
	IEventListener* mListener;
};
#endif // __MENU_BUTTON_H__
