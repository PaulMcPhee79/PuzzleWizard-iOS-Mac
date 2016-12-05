#ifndef __IEVENT_LISTENER_H__
#define __IEVENT_LISTENER_H__

class IEventListener
{
public:
	virtual ~IEventListener(void) { }
	virtual void onEvent(int evType, void* evData) = 0;
};
#endif // __IEVENT_LISTENER_H__
