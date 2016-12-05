
#include "InputManager.h"
#include <Managers/ControlsManager.h>
#include <algorithm>

using namespace CMInputs;

static InputManager *g_sharedInputManager = NULL;

InputManager* InputManager::IM(void)
{
	if (!g_sharedInputManager)
	{
		g_sharedInputManager = new InputManager();
	}
	return g_sharedInputManager;
}

InputManager::InputManager(void)
:
mEnabled(true),
mBusyUpdatingClients(false),
mFocusMap(0),
mModalFocusMap(0),
mControllerState(0),
mIsDepressedVectorDirty(true),
mIsHeldVectorDirty(true),
mDepressedVector(CMCoordZero),
mHeldVector(CMCoordZero)
{
	CCAssert(g_sharedInputManager == NULL, "Attempted to allocate a second instance of a singleton.");
    
#ifndef __ANDROID__
    mKeyMap[kVK_ANSI_W] = CI_UP;
    mKeyMap[kVK_ANSI_S] = CI_DOWN;
    mKeyMap[kVK_ANSI_A] = CI_LEFT;
    mKeyMap[kVK_ANSI_D] = CI_RIGHT;
    mKeyMap[kVK_ANSI_Comma] = CI_PREV_SONG;
    mKeyMap[kVK_ANSI_Period] = CI_NEXT_SONG;
    mKeyMap[kVK_UpArrow] = CI_UP;
    mKeyMap[kVK_DownArrow] = CI_DOWN;
    mKeyMap[kVK_LeftArrow] = CI_LEFT;
    mKeyMap[kVK_RightArrow] = CI_RIGHT;
    mKeyMap[kVK_ANSI_Keypad8] = CI_UP;
    mKeyMap[kVK_ANSI_Keypad2] = CI_DOWN;
    mKeyMap[kVK_ANSI_Keypad4] = CI_LEFT;
    mKeyMap[kVK_ANSI_Keypad6] = CI_RIGHT;
    mKeyMap[kVK_ANSI_KeypadEnter] = CI_CONFIRM;
    mKeyMap[kVK_Return] = CI_CONFIRM;
    mKeyMap[kVK_Space] = CI_CONFIRM;
    mKeyMap[kVK_Escape] = CI_CANCEL;
    mKeyMap[kVK_Control] = CI_CTRL;
    mKeyMap[kVK_Command] = CI_CMD;
    mKeyMap[kVK_ANSI_F] = CI_FULLSCREEN;
    
    const int kKeyCount = 21;
    usint keys[kKeyCount] = {
        kVK_ANSI_W,
        kVK_ANSI_S,
        kVK_ANSI_A,
        kVK_ANSI_D,
        kVK_ANSI_Comma,
        kVK_ANSI_Period,
        kVK_UpArrow,
        kVK_DownArrow,
        kVK_LeftArrow,
        kVK_RightArrow,
        kVK_ANSI_Keypad8,
        kVK_ANSI_Keypad2,
        kVK_ANSI_Keypad4,
        kVK_ANSI_Keypad6,
        kVK_ANSI_KeypadEnter,
        kVK_Return,
        kVK_Space,
        kVK_Escape,
        kVK_Control,
        kVK_Command,
        kVK_ANSI_F
    };
    
    ControlsManager::CM()->registerKeys(keys, kKeyCount);
#endif
    
    pushFocusState(FOCUS_STATE_NONE);
    pushFocusState(FOCUS_STATE_NONE, true);
}

bool InputManager::contains(Interactables* clients, IInteractable* client)
{
    if (clients && client)
    {
        Interactables::iterator findIt = std::find(clients->begin(), clients->end(), client);
        return findIt != clients->end();
    }
    else
        return false;
}

void InputManager::add(Interactables* clients, IInteractable* client)
{
    if (clients && client && !contains(clients, client))
        clients->push_back(client);
}

void InputManager::remove(Interactables* clients, IInteractable* client)
{
    if (contains(clients, client))
        clients->erase(std::remove(clients->begin(), clients->end(), client));
}

void InputManager::subscribe(IInteractable* client, bool modal)
{
#ifdef CHEEKY_DESKTOP
    if (client == NULL)
        return;
    
    Interactables* clients = modal ? &mModalClients : &mClients;
    Interactables* subscribeQueue = modal ? &mModalSubscribeQueue : &mSubscribeQueue;
    Interactables* unsubscribeQueue = modal ? &mModalUnsubscribeQueue : &mUnsubscribeQueue;
    
    if (mBusyUpdatingClients)
    {
        if (!contains(clients, client))
            add(subscribeQueue, client);
        
        remove(unsubscribeQueue, client);
    }
    else
    {
        add(clients, client);
    }
    
    if (hasFocus(client->getInputFocus()))
        client->didGainFocus();
    else
        client->willLoseFocus();
#endif
}

void InputManager::unsubscribe(IInteractable* client, bool modal)
{
#ifdef CHEEKY_DESKTOP
    if (client == NULL)
        return;
    
    Interactables* clients = modal ? &mModalClients : &mClients;
    Interactables* subscribeQueue = modal ? &mModalSubscribeQueue : &mSubscribeQueue;
    Interactables* unsubscribeQueue = modal ? &mModalUnsubscribeQueue : &mUnsubscribeQueue;
    
    if (mBusyUpdatingClients)
    {
        add(unsubscribeQueue, client);
        remove(subscribeQueue, client);
    }
    else
        remove(clients, client);
#endif
}

int InputManager::pollControllerInputs(void)
{
    int controllerState = 0;
    ControlsManager* cm = ControlsManager::CM();
    
    for (std::map<usint, ControllerInputs>::iterator it = mKeyMap.begin(); it != mKeyMap.end(); ++it)
    {
        if (cm->isKeyDown(it->first))
            controllerState |= it->second;
    }
    
    return controllerState;
}

void InputManager::update(void)
{
#ifdef CHEEKY_DESKTOP
    if (!isEnabled())
        return;
    
    mIsDepressedVectorDirty = mIsHeldVectorDirty = true;
    
    mControllerState = pollControllerInputs();
    Interactables* clients = mModalFocusMap != 0 ? &mModalClients : &mClients;
    
    mBusyUpdatingClients = true;
    for (Interactables::iterator it = clients->begin(); it != clients->end(); ++it)
    {
        if (hasFocus((*it)->getInputFocus()))
        {
            (*it)->update(mControllerState);
            if (!hasFocus((*it)->getInputFocus()))
                break;
        }
    }
    mBusyUpdatingClients = false;
    
    for (Interactables::iterator it = mSubscribeQueue.begin(); it != mSubscribeQueue.end(); ++it)
        add(&mClients, *it);
    for (Interactables::iterator it = mUnsubscribeQueue.begin(); it != mUnsubscribeQueue.end(); ++it)
        remove(&mClients, *it);
    mSubscribeQueue.clear();
    mUnsubscribeQueue.clear();
    
    for (Interactables::iterator it = mModalSubscribeQueue.begin(); it != mModalSubscribeQueue.end(); ++it)
        add(&mClients, *it);
    for (Interactables::iterator it = mModalUnsubscribeQueue.begin(); it != mModalUnsubscribeQueue.end(); ++it)
        remove(&mClients, *it);
    mModalSubscribeQueue.clear();
    mModalUnsubscribeQueue.clear();
#endif
}

void InputManager::updateFocusMap(uint focusState, bool modal)
{
    uint focusMap = 0;
    
    switch (focusState)
    {
        case FOCUS_STATE_NONE:
            focusMap = 0;
            break;
        case FOCUS_STATE_TITLE:
            focusMap = HAS_FOCUS_TITLE;
            break;
        case FOCUS_STATE_MENU:
            focusMap = HAS_FOCUS_MENU;
            break;
        case FOCUS_STATE_PUZZLE_MENU:
            focusMap = HAS_FOCUS_PUZZLE_MENU;
            break;
        case FOCUS_STATE_MENU_DIALOG:
            focusMap = HAS_FOCUS_MENU_DIALOG;
            break;
        case FOCUS_STATE_PF_PLAYFIELD:
            focusMap = HAS_FOCUS_BOARD;
            break;
        default:
            break;
    }
    
    notifyFocusChange(focusMap, modal);
    
    if (modal)
        mModalFocusMap = focusMap;
    else
        mFocusMap = focusMap;
}

bool InputManager::hasFocus(uint focus) const
{
    uint focusMap = mModalFocusMap != 0 ? mModalFocusMap : mFocusMap;
    return hasFocus(focusMap, focus);
}

bool InputManager::hasFocus(uint focusMap, uint focus) const
{
    // This is for incremental FOCUS_CAT values
    //return (focusMap & FOCUS_CAT_MASK) == (focus & FOCUS_CAT_MASK) && ((focusMap & HAS_FOCUS_MASK) & (focus & HAS_FOCUS_MASK)) != 0;
    
    // This is for bitmask FOCUS_CAT values
    return ((focusMap & FOCUS_CAT_MASK) & (focus & FOCUS_CAT_MASK)) != 0 && ((focusMap & HAS_FOCUS_MASK) & (focus & HAS_FOCUS_MASK)) != 0;
}

void InputManager::notifyFocusChange(uint focusMap, bool modal)
{
    bool wasBusy = mBusyUpdatingClients;
    
    mBusyUpdatingClients = true;
    for (int i = 0; i < 2; ++i)
    {
        // Non-modal clients don't need to know about:
            // 1. Non-modal focus states if the modal focus map is active.
            // 2. Modal focus states if they don't toggle the modal focus map activity (on/off).
        if (i == 0 && ((!modal && mModalFocusMap != 0) || (modal && focusMap != 0 && mModalFocusMap != 0)))
            continue;
        
        // Modal clients don't need to know about non-modal focus changes.
        if (i == 1 && !modal)
            continue;
        
        uint oldFocusMap = 0, newFocusMap = 0;
        
        if (i == 0)
        {
            if (modal && focusMap == 0 && mModalFocusMap != 0)
            {
                // Special case: switching from modal back down to non-modal
                oldFocusMap = focusMap;
                newFocusMap = mFocusMap;
            }
            else
            {
                oldFocusMap = mFocusMap;
                newFocusMap = focusMap;
            }
        }
        else
        {
            oldFocusMap = mModalFocusMap;
            newFocusMap = focusMap;
        }
        
        Interactables* clients = i == 0 ? &mClients : &mModalClients;
        for (Interactables::iterator it = clients->begin(); it != clients->end(); ++it)
        {
            bool didHaveFocus = hasFocus(oldFocusMap, (*it)->getInputFocus());
            bool doesHaveFocus = hasFocus(newFocusMap, (*it)->getInputFocus());
            
            if (didHaveFocus && !doesHaveFocus)
                (*it)->willLoseFocus();
            else if (!didHaveFocus && doesHaveFocus)
                (*it)->didGainFocus();
        }
        
        clients = i == 0 ? &mSubscribeQueue : &mModalSubscribeQueue;
        for (Interactables::iterator it = clients->begin(); it != clients->end(); ++it)
        {
            bool didHaveFocus = hasFocus(oldFocusMap, (*it)->getInputFocus());
            bool doesHaveFocus = hasFocus(newFocusMap, (*it)->getInputFocus());
            
            if (didHaveFocus && !doesHaveFocus)
                (*it)->willLoseFocus();
            else if (!didHaveFocus && doesHaveFocus)
                (*it)->didGainFocus();
        }
    }
    
    if (!wasBusy)
        mBusyUpdatingClients = false;
}

void InputManager::pushFocusState(uint focusState, bool modal)
{
#ifdef CHEEKY_DESKTOP
    std::vector<uint>* focusStack = modal ? &mModalFocusStack : &mFocusStack;
    int stackCount = focusStack->size();
    
    // Don't allow the same state to double-up on top of the stack. This would only happen
    // when clients are mismanaging states.
    if (stackCount == 0 || (*focusStack)[stackCount - 1] != focusState)
    {
        focusStack->push_back(focusState);
        updateFocusMap(focusState, modal);
    }
#endif
}

void InputManager::popFocusState(uint focusState, bool modal)
{
#ifdef CHEEKY_DESKTOP
    std::vector<uint>* focusStack = modal ? &mModalFocusStack : &mFocusStack;
    int stackCount = focusStack->size();
    
    if (stackCount > 1) // Don't pop base state
    {
        if (focusState == FOCUS_STATE_NONE || (*focusStack)[stackCount - 1] == focusState)
        {
            focusStack->erase(focusStack->begin() + (stackCount - 1));
            updateFocusMap((*focusStack)[stackCount - 2], modal);
        }
    }
#endif
}

void InputManager::popToFocusState(uint focusState, bool modal)
{
#ifdef CHEEKY_DESKTOP
    std::vector<uint>* focusStack = modal ? &mModalFocusStack : &mFocusStack;
    while (focusStack->size() > 1 && (*focusStack)[focusStack->size() - 1] != focusState)
        popFocusState((*focusStack)[focusStack->size() - 1]);
    
    // If focus state was not on the stack, then push it onto the stack.
    if (focusStack->size() == 1)
        pushFocusState(focusState);
    else
        updateFocusMap(focusState, modal);
#endif
}

Coord InputManager::getDepressedVector(void)
{
#ifndef __ANDROID__
    if (mIsDepressedVectorDirty)
    {
        mIsDepressedVectorDirty = false;
        mDepressedVector = CMCoordZero;
        
        ControlsManager* cm = ControlsManager::CM();
        if (cm->didKeyDepress(kVK_LeftArrow) || cm->didKeyDepress(kVK_ANSI_A) || cm->didKeyDepress(kVK_ANSI_Keypad4))
            mDepressedVector.x = -1;
        else if (cm->didKeyDepress(kVK_RightArrow) || cm->didKeyDepress(kVK_ANSI_D) || cm->didKeyDepress(kVK_ANSI_Keypad6))
            mDepressedVector.x = 1;
        else if (cm->didKeyDepress(kVK_UpArrow) || cm->didKeyDepress(kVK_ANSI_W) || cm->didKeyDepress(kVK_ANSI_Keypad8))
            mDepressedVector.y = -1;
        else if (cm->didKeyDepress(kVK_DownArrow) || cm->didKeyDepress(kVK_ANSI_S) || cm->didKeyDepress(kVK_ANSI_Keypad2))
            mDepressedVector.y = 1;
    }
#endif
    
    return mDepressedVector;
}

Coord InputManager::getHeldVector(void)
{
#ifndef __ANDROID__
    if (mIsHeldVectorDirty)
    {
        mIsHeldVectorDirty = false;
        mHeldVector = CMCoordZero;
        
        ControlsManager* cm = ControlsManager::CM();
        if (cm->isKeyDown(kVK_LeftArrow) || cm->isKeyDown(kVK_ANSI_A) || cm->isKeyDown(kVK_ANSI_Keypad4))
            mHeldVector.x = -1;
        else if (cm->isKeyDown(kVK_RightArrow) || cm->isKeyDown(kVK_ANSI_D) || cm->isKeyDown(kVK_ANSI_Keypad6))
            mHeldVector.x = 1;
        else if (cm->isKeyDown(kVK_UpArrow) || cm->isKeyDown(kVK_ANSI_W) || cm->isKeyDown(kVK_ANSI_Keypad8))
            mHeldVector.y = -1;
        else if (cm->isKeyDown(kVK_DownArrow) || cm->isKeyDown(kVK_ANSI_S) || cm->isKeyDown(kVK_ANSI_Keypad2))
            mHeldVector.y = 1;
    }
#endif
    
    return mHeldVector;
}

Coord InputManager::getMovementVector(int controllerState)
{
    Coord movement;
    
    if ((controllerState & CI_LEFT) == CI_LEFT)
        movement.x = -1;
    else if ((controllerState & CI_RIGHT) == CI_RIGHT)
        movement.x = 1;
    else if ((controllerState & CI_UP) == CI_UP)
        movement.y = -1;
    else if ((controllerState & CI_DOWN) == CI_DOWN)
        movement.y = 1;
    
    return movement;
}
