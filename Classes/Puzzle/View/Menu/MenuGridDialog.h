#ifndef __MENU_GRID_DIALOG_H__
#define __MENU_GRID_DIALOG_H__

#include "cocos2d.h"
#include <Puzzle/View/Menu/MenuDialog.h>
USING_NS_CC;

class MenuGridDialog : public MenuDialog
{
public:
	MenuGridDialog(int category, int priority, uint inputFocus, int columnLen);
	virtual ~MenuGridDialog(void);

	static MenuGridDialog* create(int category, int priority, uint inputFocus, int columnLen, IEventListener* listener = NULL, bool autorelease = true);
	virtual bool init(void);
    
    virtual void resetNav(void);

    virtual void update(int controllerState);
    virtual void advanceTime(float dt);

private:
    int getColumnLen(void) const { return mColumnLen; }
    int getNumColumns(void) const { return getColumnLen() == 0 ? 1 : getNavCountMinusOne() / getColumnLen() + MIN(1, getNavCountMinusOne() & getColumnLen()); }
    int getNavCount(void) const { return mButtonsProxy.getNavCount(); }
    int getNavCountMinusOne(void) const { return getNavCount() - 1; }
    int getInterceptIndex(void) const { return getNavCountMinusOne(); }
    bool isInterceptingInput(void) const { return mIsInterceptingInput; }
    void setInterceptingInput(bool value) { mIsInterceptingInput = value; }
    
    void processNavInput(int prevNavIndex);
    void navigateToIndex(int navIndex);
    bool isTopRow(int navIndex);
    bool isBtmRow(int navIndex);
    int didNavigateHorizontally(void);
    int didNavigateVertically(void);
    
	bool mIsInterceptingInput;
    int mColumnLen;
    int mPrevNavIndex;
    CCPoint mPrevNav;
};
#endif // __MENU_GRID_DIALOG_H__
