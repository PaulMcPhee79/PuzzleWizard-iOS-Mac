
#include "ButtonsProxy.h"
#include <Prop/UIControls/MenuButton.h>
#include <Managers/ControlsManager.h>
#include <Utils/Utils.h>

ButtonsProxy::ButtonsProxy(uint inputFocus, NavigationMap navMap)
:
INavigable(navMap),
mUINavigator(navMap),
mInputFocus(inputFocus),
mFocusDeselectedButton(NULL),
mLocked(false)
{
    
}

ButtonsProxy::~ButtonsProxy(void)
{
    
}

MenuButton* ButtonsProxy::getSelectedButton(void)
{
    MenuButton* selectedButton = NULL;
    for (std::vector<MenuButton*>::iterator it = mButtons.begin(); it != mButtons.end(); ++it)
    {
        if ((*it)->isSelected())
        {
            selectedButton = *it;
            break;
        }
    }
    
    return selectedButton;
}

void ButtonsProxy::resetNav(void)
{
    mUINavigator.resetNav();
}

void ButtonsProxy::movePrevNav(void)
{
    mUINavigator.movePrevNav();
}

void ButtonsProxy::moveNextNav(void)
{
    mUINavigator.moveNextNav();
}

void ButtonsProxy::addButton(MenuButton* menuButton)
{
    if (menuButton == NULL)
        return;
    
    if (!mLocked)
    {
        if (!CMUtils::contains(mButtons, menuButton))
        {
            mButtons.push_back(menuButton);
            if (menuButton->isNavigable())
                mUINavigator.addNav(menuButton);
        }
    }
    else
    {
        if (!CMUtils::contains(mAddQueue, menuButton))
        {
            CMUtils::erase(mRemoveQueue, menuButton);
            mAddQueue.push_back(menuButton);
        }
    }
}

void ButtonsProxy::removeButton(MenuButton* menuButton)
{
    if (menuButton == NULL)
        return;
    
    if (!mLocked)
    {
        menuButton->setSelected(false);
        CMUtils::erase(mButtons, menuButton);
        mUINavigator.removeNav(menuButton);
        if (menuButton == mFocusDeselectedButton)
            mFocusDeselectedButton = NULL;
    }
    else
    {
        if (!CMUtils::contains(mRemoveQueue, menuButton))
        {
            CMUtils::erase(mAddQueue, menuButton);
            mRemoveQueue.push_back(menuButton);
        }
    }
}

void ButtonsProxy::clear(void)
{
    for (int i = (int)mButtons.size() - 1; i >= 0; --i)
        removeButton(mButtons[i]);
    mFocusDeselectedButton = NULL;
}

void ButtonsProxy::didGainFocus(void)
{
    if (mFocusDeselectedButton && mFocusDeselectedButton == getCurrentNav())
        mFocusDeselectedButton->setSelected(true);
    mFocusDeselectedButton = NULL;
}

void ButtonsProxy::willLoseFocus(void)
{
    MenuButton* selectedButton = getSelectedButton();
    if (selectedButton)
    {
        selectedButton->setSelected(false);
        mFocusDeselectedButton = selectedButton;
    }
    else
        mFocusDeselectedButton = NULL;
}

void ButtonsProxy::update(int controllerState)
{
    ControlsManager* cm = ControlsManager::CM();
    
    mLocked = true;
    for (std::vector<MenuButton*>::iterator it = mButtons.begin(); it != mButtons.end(); ++it)
    {
        MenuButton* menuButton = *it;
        if (menuButton->isSelected())
        {
            if (cm->didAccept(true))
                menuButton->depress();
            else if (cm->didAccept(false))
                menuButton->raise();
        }
        else
        {
            menuButton->raise();
        }
    }
    mLocked = false;
    
    if (mAddQueue.size() > 0)
    {
        for (std::vector<MenuButton*>::iterator it = mAddQueue.begin(); it != mAddQueue.end(); ++it)
            addButton(*it);
        mAddQueue.clear();
    }
    
    if (mRemoveQueue.size() > 0)
    {
        for (std::vector<MenuButton*>::iterator it = mRemoveQueue.begin(); it != mRemoveQueue.end(); ++it)
            removeButton(*it);
        mRemoveQueue.clear();
    }
    
    mUINavigator.update(controllerState);
}

void ButtonsProxy::advanceTime(float dt)
{
    mUINavigator.advanceTime(dt);
}
