
#ifndef __CONTROLS_MANAGER__
#define __CONTROLS_MANAGER__

#include "cocos2d.h"
#ifdef CHEEKY_DESKTOP
#import <Carbon/Carbon.h>
#endif
USING_NS_CC;

typedef unsigned short usint;
typedef std::map<usint, bool> CMKeyCodes;

class ControlsManager
{
public:
#ifdef CHEEKY_MOBILE
    #define kVK_ANSI_W 0
    #define kVK_ANSI_S 1
    #define kVK_ANSI_A 2
    #define kVK_ANSI_D 3
    #define kVK_ANSI_Comma 4
    #define kVK_ANSI_Period 5
    #define kVK_UpArrow 6
    #define kVK_DownArrow 7
    #define kVK_LeftArrow 8
    #define kVK_RightArrow 9
    #define kVK_ANSI_Keypad8 10
    #define kVK_ANSI_Keypad2 11
    #define kVK_ANSI_Keypad4 12
    #define kVK_ANSI_Keypad6 13
    #define kVK_ANSI_KeypadEnter 14
    #define kVK_Return 15
    #define kVK_Space 16
    #define kVK_Escape 17
    #define kVK_Delete 18
    #define kVK_Control 19
    #define kVK_Command 20
    #define kVK_ANSI_F 21
#endif
    
	static ControlsManager* CM(void);

    void registerKey(usint key);
    void registerKeys(usint keys[], int keyCount);
    void unregisterKey(usint key);
    void unregisterKeys(usint keys[], int keyCount);
    
    void clearKeyStates(void);
    
    bool isKeyDown(usint key);
    bool isKeyUp(usint key);
    bool didKeyDepress(usint key);
    bool didKeyRelease(usint key);
    
    bool didAccept(bool depressed);
    bool didCancel(bool depressed);
    bool didToggleFullscreen(bool depressed);
    
    void keyDown(usint key);
    void keyUp(usint key);
    void update(void);
    
private:
    ControlsManager();
    bool isKeyRegistered(usint key);
    bool wasKeyDown(usint key);
    bool wasKeyUp(usint key);
    
    CMKeyCodes mKeyStates;
    CMKeyCodes mPrevKeyStates;
};

#endif
