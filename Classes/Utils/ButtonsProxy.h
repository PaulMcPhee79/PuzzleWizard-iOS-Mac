#ifndef __BUTTONS_PROXY_H__
#define __BUTTONS_PROXY_H__

#include "cocos2d.h"
#include <Interfaces/IInteractable.h>
#include <Interfaces/INavigable.h>
#include <Utils/UINavigator.h>
class MenuButton;
USING_NS_CC;

class ButtonsProxy : public CCObject, public IInteractable, public INavigable
{
public:
	ButtonsProxy(uint inputFocus, NavigationMap navMap);
	virtual ~ButtonsProxy(void);

    bool doesRepeat(void) const { return mUINavigator.doesRepeat(); }
    void setRepeats(bool value) { mUINavigator.setRepeats(value); }
    float getRepeatDelay(void) const { return mUINavigator.getRepeatDelay(); }
    void setRepeatDelay(float value) { mUINavigator.setRepeatDelay(value); }
    int getNavIndex(void) const { return mUINavigator.getNavIndex(); }
    int getNavCount(void) const { return mUINavigator.getNavCount(); }
    
    virtual uint getInputFocus(void) const { return mInputFocus; }
    void setInputFocus(uint value) { mInputFocus = value; }
    MenuButton* getSelectedButton(void);
    
    CCNode* getCurrentNav(void) const { return mUINavigator.getCurrentNav(); }
    void resetNav(void);
    void movePrevNav(void);
    void moveNextNav(void);
    void addButton(MenuButton* menuButton);
    void removeButton(MenuButton* menuButton);
    void clear(void);
    
    virtual void didGainFocus(void);
    virtual void willLoseFocus(void);
    virtual void update(int controllerState);
    void advanceTime(float dt);
    
private:
    UINavigator mUINavigator;
    uint mInputFocus;
    MenuButton* mFocusDeselectedButton;
    std::vector<MenuButton*> mButtons;
    
    bool mLocked;
    std::vector<MenuButton*> mAddQueue;
    std::vector<MenuButton*> mRemoveQueue;

};
#endif // __BUTTONS_PROXY_H__
