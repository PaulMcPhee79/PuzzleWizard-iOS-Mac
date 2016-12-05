#ifndef __IINTERACTABLE_H__
#define __IINTERACTABLE_H__

enum ControllerInputs
{
    CI_UP = 1<<0,
    CI_DOWN = 1<<1,
    CI_LEFT = 1<<2,
    CI_RIGHT = 1<<3,
    CI_CONFIRM = 1<<4,
    CI_CANCEL = 1<<5,
    CI_PREV_SONG = 1<<6,
    CI_NEXT_SONG = 1<<7,
    CI_CTRL = 1<<8,
    CI_CMD = 1<<9,
    CI_FULLSCREEN = 1<<10
};

class IInteractable
{
public:
	virtual ~IInteractable(void) { }
    virtual unsigned int getInputFocus(void) const = 0;
	virtual void didGainFocus(void) { }
    virtual void willLoseFocus(void) { }
    virtual void update(int controllerState) { }
};
#endif // __IINTERACTABLE_H__
