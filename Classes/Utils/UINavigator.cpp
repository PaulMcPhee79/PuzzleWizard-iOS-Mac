
#include "UINavigator.h"
#include <Prop/UIControls/MenuButton.h>
#include <Managers/InputManager.h>
#include <Utils/CMTypes.h>
#include <Utils/Utils.h>
USING_NS_CC;

UINavigator::UINavigator(NavigationMap navMap)
:
INavigable(navMap),
mRepeats(false),
mRepeatDir(0),
mRepeatCounter(0),
mRepeatDelay(0.25f),
mNavIndex(0)
{

}

UINavigator::~UINavigator(void)
{
    
}

void UINavigator::activateNav(CCNode* nav)
{
    if (nav)
    {
        MenuButton* button = dynamic_cast<MenuButton*>(nav);
        if (button)
            button->setSelected(true);
    }
}

void UINavigator::deactivateNav(CCNode* nav)
{
    if (nav)
    {
        MenuButton* button = dynamic_cast<MenuButton*>(nav);
        if (button)
            button->setSelected(false);
    }
}

void UINavigator::addNav(CCNode* nav)
{
    if (nav && !CMUtils::contains(mNavigables, nav))
    {
        mNavigables.push_back(nav);
        
        if (mNavigables.size() == 1)
            activateNav(nav);
    }
}

void UINavigator::removeNav(CCNode* nav)
{
    if (nav)
    {
        if (nav == getCurrentNav())
        {
            deactivateNav(nav);
            CMUtils::erase(mNavigables, nav);
            movePrevNav();
        }
        else
            CMUtils::erase(mNavigables, nav);
    }
}

void UINavigator::resetNav(void)
{
    deactivateNav(getCurrentNav());
    mNavIndex = 0;
    
    CCNode* currentNav = getCurrentNav();
    if (currentNav && currentNav->isVisible())
        activateNav(currentNav);
    else
        activateNextActiveNav(1);
}

// Skips invisible navs
void UINavigator::activateNextActiveNav(int dir)
{
    CCNode* currentNav = getCurrentNav();
    if (currentNav && mNavigables.size() > 0 && (dir == 1 || dir == -1))
    {
        int startIndex = CMUtils::indexOf(mNavigables, currentNav);
        if (startIndex == -1)
            return;
        
        int i = startIndex + dir;
        while ( (i != startIndex))
        {
            if (dir == 1)
                moveNextNav();
            else
                movePrevNav();
            
            CCNode* nav = getCurrentNav();
            if (nav && nav->isVisible())
                break;
            
            if (i < 0)
            {
                i = (int)mNavigables.size() - 1;
                continue;
            }
            else if (i >= mNavigables.size())
            {
                i = 0;
                continue;
            }
            
            i += dir;
        }
    }
}

void UINavigator::movePrevNav(void)
{
    if (mNavigables.size() > 0)
    {
        deactivateNav(getCurrentNav());
        --mNavIndex;
        
        if (mNavIndex < 0)
            mNavIndex = (int)mNavigables.size() - 1;
        
        activateNav(getCurrentNav());
    }
}

void UINavigator::moveNextNav(void)
{
    if (mNavigables.size() > 0)
    {
        deactivateNav(getCurrentNav());
        ++mNavIndex;
        
        if (mNavIndex >= mNavigables.size())
            mNavIndex = 0;
        
        activateNav(getCurrentNav());
    }
}

void UINavigator::update(int controllerState)
{
    InputManager* im = InputManager::IM();
    Coord depressedVec = im->getDepressedVector(), heldVec = im->getHeldVector();
    int didNavigate = 0, didRepeat = 0;
    
    if ((mNavMap & NAV_VERT) == NAV_VERT)
    {
        if (depressedVec.y == -1)
        {
            movePrevNav();
            didNavigate = -1;
        }
        else if (depressedVec.y == 1)
        {
            moveNextNav();
            didNavigate = 1;
        }
        
        if (doesRepeat() && didNavigate == 0)
        {
            if (heldVec.y == -1)
            {
                setRepeatDir(-1);
                didRepeat = -1;
            }
            else if (heldVec.y == 1)
            {
                setRepeatDir(1);
                didRepeat = 1;
            }
        }
    }
    
    if (didNavigate == 0 && (mNavMap & NAV_HORIZ) == NAV_HORIZ)
    {
        if (depressedVec.x == -1)
        {
            movePrevNav();
            didNavigate = -1;
        }
        else if (depressedVec.x == 1)
        {
            moveNextNav();
            didNavigate = 1;
        }
        
        if (doesRepeat() && didRepeat == 0 && didNavigate == 0)
        {
            if (heldVec.x == -1)
            {
                setRepeatDir(-1);
                didRepeat = -1;
            }
            else if (heldVec.x == 1)
            {
                setRepeatDir(1);
                didRepeat = 1;
            }
        }
    }
    
    if (didRepeat == 0 || didNavigate != 0)
        setRepeatDir(0);
    
    // Skip invisible navs
    if (didNavigate != 0 && getCurrentNav() && !getCurrentNav()->isVisible())
        activateNextActiveNav(didNavigate);
}

void UINavigator::advanceTime(float dt)
{
    if (doesRepeat() && getRepeatDir() != 0)
    {
        mRepeatCounter -= dt;
        if (mRepeatCounter < 0)
        {
            mRepeatCounter = mRepeatDelay;
            
            if (getRepeatDir() == -1)
                movePrevNav();
            else
                moveNextNav();
            
            CCNode* currentNav = getCurrentNav();
            if (currentNav && !currentNav->isVisible())
                activateNextActiveNav(getRepeatDir());
        }
    }
}
