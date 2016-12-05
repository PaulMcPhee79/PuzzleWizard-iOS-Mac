
#include "ControlsManager.h"

static ControlsManager *g_sharedControlsManager = NULL;

ControlsManager* ControlsManager::CM(void)
{
	if (!g_sharedControlsManager)
	{
		g_sharedControlsManager = new ControlsManager();
	}
	return g_sharedControlsManager;
}

ControlsManager::ControlsManager(void)
{
	CCAssert(g_sharedControlsManager == NULL, "Attempted to allocate a second instance of a singleton.");
}

bool ControlsManager::isKeyRegistered(usint key)
{
    return mKeyStates.count(key) != 0;
}

void ControlsManager::registerKey(usint key)
{
    if (!isKeyRegistered(key))
    {
        mKeyStates[key] = false;
        mPrevKeyStates[key] = false;
    }
}

void ControlsManager::registerKeys(usint keys[], int keyCount)
{
    for (int i = 0; i < keyCount; ++i)
        registerKey(keys[i]);
}

void ControlsManager::unregisterKey(usint key)
{
    if (isKeyRegistered(key))
    {
        mKeyStates.erase(key);
        mPrevKeyStates.erase(key);
    }
}

void ControlsManager::unregisterKeys(usint keys[], int keyCount)
{
    for (int i = 0; i < keyCount; ++i)
        unregisterKey(keys[i]);
}

void ControlsManager::clearKeyStates(void)
{
    for (CMKeyCodes::iterator it = mKeyStates.begin(); it != mKeyStates.end(); ++it)
        it->second = false;
    for (CMKeyCodes::iterator it = mPrevKeyStates.begin(); it != mPrevKeyStates.end(); ++it)
        it->second = false;
}

bool ControlsManager::isKeyDown(usint key)
{
    return isKeyRegistered(key) ? mKeyStates[key] : false;
}

bool ControlsManager::isKeyUp(usint key)
{
    return isKeyRegistered(key) ? !mKeyStates[key] : true;
}

bool ControlsManager::wasKeyDown(usint key)
{
    return isKeyRegistered(key) ? mPrevKeyStates[key] : false;
}

bool ControlsManager::wasKeyUp(usint key)
{
    return isKeyRegistered(key) ? !mPrevKeyStates[key] : true;
}

bool ControlsManager::didKeyDepress(usint key)
{
    return wasKeyUp(key) && isKeyDown(key);
}

bool ControlsManager::didKeyRelease(usint key)
{
    return wasKeyDown(key) && isKeyUp(key);
}

bool ControlsManager::didAccept(bool depressed)
{
#ifndef __ANDROID__
    return depressed
        ? didKeyDepress(kVK_Return) || didKeyDepress(kVK_Space) || didKeyDepress(kVK_ANSI_KeypadEnter)
        : didKeyRelease(kVK_Return) || didKeyRelease(kVK_Space) || didKeyRelease(kVK_ANSI_KeypadEnter);
#else
    return false;
#endif
}

bool ControlsManager::didCancel(bool depressed)
{
#ifndef __ANDROID__
    return depressed ? didKeyDepress(kVK_Escape) : didKeyRelease(kVK_Escape);
#else
    return false;
#endif
}

bool ControlsManager::didToggleFullscreen(bool depressed)
{
//#ifdef CHEEKY_DESKTOP
//    return depressed
//    ? isKeyDown(kVK_Control) && isKeyDown(kVK_Command) && didKeyDepress(kVK_ANSI_F)
//    : isKeyDown(kVK_Control) && isKeyDown(kVK_Command) && didKeyRelease(kVK_ANSI_F);
//#else
//    return false;
//#endif
    
    // cocos2dx doesn't seem to recognize kVK_Control or kVK_Command
    return false;
}

void ControlsManager::keyDown(usint key)
{
    if (isKeyRegistered(key) && !mKeyStates[key])
    {
        mPrevKeyStates[key] = mKeyStates[key];
        mKeyStates[key] = true;
    }
}

void ControlsManager::keyUp(usint key)
{
    if (isKeyRegistered(key) && mKeyStates[key])
    {
        mPrevKeyStates[key] = mKeyStates[key];
        mKeyStates[key] = false;
    }
}

void ControlsManager::update(void)
{
#ifdef CHEEKY_DESKTOP
    for (CMKeyCodes::iterator it = mKeyStates.begin(); it != mKeyStates.end(); ++it)
        mPrevKeyStates[it->first] = it->second;
#endif
}

