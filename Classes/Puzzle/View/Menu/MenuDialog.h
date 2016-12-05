#ifndef __MENU_DIALOG_H__
#define __MENU_DIALOG_H__

#include "cocos2d.h"
#include <Prop/Prop.h>
#include <Interfaces/IEventListener.h>
#include <Puzzle/View/Menu/MenuItem/MenuItem.h>
#include <Utils/ButtonsProxy.h>
#include <Events/EventDispatcher.h>
#include <Interfaces/IResDependent.h>
#include <Interfaces/ILocalizable.h>
class FloatTweener;
USING_NS_CC;

typedef void(*CUSTOM_LAYOUT_FUNC)(int uid, const std::vector<CCNode*>& layoutItems);

class MenuDialog : public Prop, public IEventListener, public IResDependent, public ILocalizable
{
public:
    enum AnimState { ANIM_NONE = 0, ZOOM_IN, ZOOM_OUT, DROP, ANIM_COUNT };
    
	static int EV_TYPE_ITEM_DID_ACTIVATE() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }

	MenuDialog(int category, int priority, uint inputFocus, NavigationMap navMap = NAV_VERT);
	virtual ~MenuDialog(void);

	static MenuDialog* create(int category, int priority, uint inputFocus, NavigationMap navMap = NAV_VERT, IEventListener* listener = NULL, bool autorelease = true);
	virtual bool init(void);

    virtual void localeDidChange(const char* fontKey, const char* FXFontKey);
    
	virtual void show(AnimState anim = ANIM_NONE);
	virtual void hide(AnimState anim = ANIM_NONE);
    virtual void resetNav(void) { mButtonsProxy.resetNav(); }
    void setShowScale(float scale);
	const char* getMenuItemTextForTag(int tag);
	void setMenuItemTextForTag(int tag, const char* text);
	void addBgItem(CCNode* item);
	void removeBgItem(CCNode* item);
	void addContentItem(CCNode* item);
	void removeContentItem(CCNode* item);
	void addMenuItem(MenuItem* item);
	void removeMenuItem(MenuItem* item);
    void addTouchableItem(IEventListener* item);
    void removeTouchableItem(IEventListener* item);
	MenuItem* getMenuItem(int tag);
    MenuItem* getMenuItem(const CCPoint& touchPos);
    MenuItem* isPressedItem(const CCPoint& touchPos);
    CCNode* getContentItem(int tag);
    virtual void resolutionDidChange(void);
	virtual void onEvent(int evType, void* evData);

	bool canGoBack(void) const { return mCanGoBack; }
	void setCanGoBack(bool value) { mCanGoBack = value; }
	bool isAnimating(void) const { return mAnimState != ANIM_NONE; }
    bool doesRepeat(void) const { return mButtonsProxy.doesRepeat(); }
    void setRepeats(bool value) { mButtonsProxy.setRepeats(value); }
    float getRepeatDelay(void) const { return mButtonsProxy.getRepeatDelay(); }
    void setRepeatDelay(float value) { mButtonsProxy.setRepeatDelay(value); }
	int getPriority(void) const { return mPriority; }
    

	IEventListener* getListener(void) const { return mListener; }
	void setListener(IEventListener* value) { mListener = value; }
    
    void enableCustomLayout(int uid, CUSTOM_LAYOUT_FUNC func)
    {
        mCustomLayoutUid = uid;
        mCustomLayoutFunc = func;
    }
    void addCustomLayoutItem(CCNode* item);
    void removeCustomLayoutItem(CCNode* item);
    void layoutCustomItems(void);
    
    virtual void update(int controllerState);
    virtual void advanceTime(float dt);
    
protected:
	static const int kZOrderBg = 1000;
	static const int kZOrderContent = 2000;
	static const int kZOrderMenuItem = 3000;
    
    AnimState getAnimState(void) const { return mAnimState; }
	void setAnimState(AnimState value);
    
    ButtonsProxy mButtonsProxy;

private:
    bool isAnimTweener(FloatTweener* tweener);
    void setPressedItem(MenuItem* item);
	void onTouch(int evType, void* evData);

	bool mCanGoBack;
    int mPriority;
    float mShowScale;
    AnimState mAnimState;
	MenuItem* mPressedItem;
    std::set<MenuItem*> mMenuItems;
    std::set<IEventListener*> mTouchableItems;

    CCNode* mBgContainer;
	CCNode* mContentContainer;
    CCNode* mMenuItemContainer;

    std::vector<FloatTweener*> mAnimTweeners;
	IEventListener* mListener;
    
    int mCustomLayoutUid;
    CUSTOM_LAYOUT_FUNC mCustomLayoutFunc;
    std::vector<CCNode*>* mCustomLayoutItems;
};
#endif // __MENU_DIALOG_H__
