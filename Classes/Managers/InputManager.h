
#ifndef __INPUT_MANAGER__
#define __INPUT_MANAGER__

#include "cocos2d.h"
#include <Interfaces/IInteractable.h>
#include "InputTypes.h"
#include <Utils/CMTypes.h>
USING_NS_CC;

typedef std::vector<IInteractable*> Interactables;

class InputManager
{    
private:
    InputManager();
    bool contains(Interactables* clients, IInteractable* client);
    void add(Interactables* clients, IInteractable* client);
    void remove(Interactables* clients, IInteractable* client);
    
    int pollControllerInputs(void);
    void updateFocusMap(uint focusState, bool modal);
    bool hasFocus(uint focusMap, uint focus) const;
    void notifyFocusChange(uint focusMap, bool modal);
    
    bool mEnabled;
    bool mBusyUpdatingClients;
    uint mFocusMap;
    std::vector<uint> mFocusStack;
    Interactables mClients;
    Interactables mSubscribeQueue;
    Interactables mUnsubscribeQueue;

    uint mModalFocusMap;
    std::vector<uint> mModalFocusStack;
    Interactables mModalClients;
    Interactables mModalSubscribeQueue;
    Interactables mModalUnsubscribeQueue;
    
    int mControllerState;
    bool mIsDepressedVectorDirty;
    bool mIsHeldVectorDirty;
    Coord mDepressedVector;
    Coord mHeldVector;
    
    std::map<unsigned short, ControllerInputs> mKeyMap;
    
public:
    static InputManager* IM(void);
    
    bool isEnabled(void) { return mEnabled; }
    void enable(bool enable) { mEnabled = enable; }
    
    void subscribe(IInteractable* client, bool modal = false);
    void unsubscribe(IInteractable* client, bool modal = false);
    void update(void);
    
    bool hasFocus(uint focus) const;
    void pushFocusState(uint focusState, bool modal = false);
    void popFocusState(uint focusState = CMInputs::FOCUS_STATE_NONE, bool modal = false);
    void popToFocusState(uint focusState, bool modal = false);
    
    Coord getDepressedVector(void);
    Coord getHeldVector(void);
    
    static Coord getMovementVector(int controllerState);
};

#endif
