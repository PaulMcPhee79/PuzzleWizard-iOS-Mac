
#include "MenuGridDialog.h"
#include <Managers/InputManager.h>
#include <Utils/Utils.h>
USING_NS_CC;

MenuGridDialog::MenuGridDialog(int category, int priority, uint inputFocus, int columnLen)
	:
	MenuDialog(category, priority, inputFocus, NAV_OMNI),
    mIsInterceptingInput(false),
	mColumnLen(MAX(1, columnLen)),
	mPrevNavIndex(0),
	mPrevNav(CCPointZero)
{

}

MenuGridDialog::~MenuGridDialog(void)
{
	// Do nothing
}

MenuGridDialog* MenuGridDialog::create(int category, int priority, uint inputFocus, int columnLen, IEventListener* listener, bool autorelease)
{
	MenuGridDialog *dialog = new MenuGridDialog(category, priority, inputFocus, columnLen);
	if (dialog && dialog->init())
    {
		dialog->setListener(listener);
		if (autorelease)
			dialog->autorelease();
        return dialog;
    }
    CC_SAFE_DELETE(dialog);
    return NULL;
}

bool MenuGridDialog::init(void)
{
	return MenuDialog::init();
}

void MenuGridDialog::resetNav(void)
{
    MenuDialog::resetNav();
    setInterceptingInput(false);
}

void MenuGridDialog::processNavInput(int prevNavIndex)
{
    int vertNav = mPrevNav.y;
    int horizNav = mPrevNav.x;
    
    if (isInterceptingInput())
    {
        if (vertNav != 0)
        {
            if (vertNav == -1)
            {
                navigateToIndex(MIN(getNavCountMinusOne() - 1, (mPrevNavIndex / getColumnLen()) * getColumnLen() + (getColumnLen() - 1)));
            }
            else
            {
                navigateToIndex((mPrevNavIndex / getColumnLen()) * getColumnLen());
            }
            
            setInterceptingInput(false);
        }
        else if (horizNav != 0)
            navigateToIndex(getInterceptIndex());
    }
    else
    {
        if (vertNav != 0)
        {
            if ((isTopRow(prevNavIndex) && vertNav == -1) || (isBtmRow(prevNavIndex) && vertNav == 1))
            {
                mPrevNavIndex = prevNavIndex;
                navigateToIndex(getInterceptIndex());
                setInterceptingInput(true);
            }
        }
        else if (horizNav != 0)
        {
            if (horizNav == -1)
            {
                int navIndex = prevNavIndex - getColumnLen();
                if (navIndex < 0)
                    navIndex = MIN(getNavCountMinusOne() - 1, navIndex + getNumColumns() * getColumnLen());
                navigateToIndex(navIndex);
            }
            else
            {
                int navIndex = prevNavIndex + getColumnLen();
                if (navIndex >= getNavCountMinusOne())
                {
                    if (getNavCountMinusOne() % getColumnLen() != 0 && prevNavIndex / getColumnLen() < getNumColumns() - 1)
                    {
                        // Penultimate column, but final column is too short. Clamp to last index.
                        navIndex = getNavCountMinusOne() - 1;
                    }
                    else
                    {
                        // Wrap around to the first column
                        navIndex = prevNavIndex % getColumnLen();
                    }
                }
                navigateToIndex(navIndex);
            }
        }
    }
}

void MenuGridDialog::navigateToIndex(int navIndex)
{
    if (navIndex >= 0 && navIndex < getNavCount())
    {
        while (mButtonsProxy.getNavIndex() != navIndex)
            mButtonsProxy.moveNextNav();
    }
}

bool MenuGridDialog::isTopRow(int navIndex)
{
    return navIndex % getColumnLen() == 0;
}

bool MenuGridDialog::isBtmRow(int navIndex)
{
    return navIndex % getColumnLen() == getColumnLen() - 1 || navIndex == getNavCountMinusOne() - 1;
}

int MenuGridDialog::didNavigateHorizontally(void)
{
    InputManager* im = InputManager::IM();
    Coord navVec = im->getDepressedVector();
    if (navVec.x == 0)
        navVec = im->getHeldVector();
    return navVec.x;
}

int MenuGridDialog::didNavigateVertically(void)
{
    InputManager* im = InputManager::IM();
    Coord navVec = im->getDepressedVector();
    if (navVec.y == 0)
        navVec = im->getHeldVector();
    return navVec.y;
}

void MenuGridDialog::update(int controllerState)
{
    int prevNavIndex = mButtonsProxy.getNavIndex();
    MenuDialog::update(controllerState);
    
    if (prevNavIndex != mButtonsProxy.getNavIndex())
    {
        mPrevNav.y = didNavigateVertically();
        mPrevNav.x = mPrevNav.y == 0 ? didNavigateHorizontally() : 0;
        processNavInput(prevNavIndex);
    }
}

void MenuGridDialog::advanceTime(float dt)
{
    if (!isInterceptingInput() || mPrevNav.y != 0)
    {
        int prevNavIndex = mButtonsProxy.getNavIndex();
        MenuDialog::advanceTime(dt);
        
        if (prevNavIndex != mButtonsProxy.getNavIndex())
        {
            mPrevNav.y = didNavigateVertically();
            mPrevNav.x = mPrevNav.y == 0 ? didNavigateHorizontally() : 0;
            processNavInput(prevNavIndex);
        }
    }
}

