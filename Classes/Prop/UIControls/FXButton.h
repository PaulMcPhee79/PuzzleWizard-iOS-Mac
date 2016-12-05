#ifndef __FX_BUTTON_H__
#define __FX_BUTTON_H__

#include "cocos2d.h"
#include <Prop/Prop.h>
#include "MenuButton.h"
#include <Events/EventDispatcher.h>
#include <Interfaces/ILocalizable.h>
#include <Utils/TextUtils.h>
class TileDecoration;
USING_NS_CC;

class FXButton : public Prop, public IEventListener, public ILocalizable
{
public:
	explicit FXButton(TextUtils::CMFontType fontType);
	virtual ~FXButton(void);
    
	static FXButton* create(TextUtils::CMFontType fontType, const char* text, int fontSize, IEventListener* listener = NULL, bool autorelease = true);
	virtual bool initWith(const char* text, int fontSize, IEventListener* listener = NULL);

    virtual void localeDidChange(const char* fontKey, const char* FXFontKey);
    
	bool isEnabled(void) const { return mButtonProxy ? mButtonProxy->isEnabled() : false; }
    bool isNavigable(void) const { return mButtonProxy ? mButtonProxy->isNavigable() : false; }
	bool isPressed(void) const { return mButtonProxy ? mButtonProxy->isPressed() : false; }
	bool isReleased(void) const { return mButtonProxy ? mButtonProxy->isReleased() : false; }
    bool isSelected(void) const { return mButtonProxy ? mButtonProxy->isSelected() : false; }
    void setSelected(bool value);
    
    void setString(const char* value);
    void setSelectedColor(uint value);
	void setUnselectedColor(uint value);
    void setSelectedArrowScale(float value);

	void enable(bool enable) { if (mButtonProxy) mButtonProxy->enable(enable); }
	void depress(void) { if (mButtonProxy) mButtonProxy->depress(); }
	void raise(void) { if (mButtonProxy) mButtonProxy->raise(); }
	void reset(void) { if (mButtonProxy) mButtonProxy->reset(); }

	void enableTouch(bool enable, CMTouches::TouchCategory category)
    {
        if (mButtonProxy)
            mButtonProxy->enableTouch(enable, category);
    }
	virtual void onEvent(int evType, void* evData);
	IEventListener* getListener(void) const { return mListener; }
	void setListener(IEventListener* value) { mListener = value; }

protected:
    virtual void repositionSelectedArrow(void);
    virtual void layoutContents(void);
    virtual void updateContentColor(void);
    virtual void updateContentBounds(void);

    TextUtils::CMFontType mFontType;
    CCRect mContentBounds;
    CCLabelBMFont* mLabel;
    TileDecoration* mSelectedArrow;
    MenuButton* mButtonProxy;
    CCNode* mContents;
    IEventListener* mListener;
    
private:
    uint mSelectedColor;
    uint mUnselectedColor;
};
#endif // __FX_BUTTON_H__
