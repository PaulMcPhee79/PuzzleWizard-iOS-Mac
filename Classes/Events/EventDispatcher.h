#ifndef __EVENT_DISPATCHER_H__
#define __EVENT_DISPATCHER_H__

#include "cocos2d.h"
#include <Interfaces/IEventListener.h>
USING_NS_CC;

class EventDispatcher
{
public:
	static int nextEvType()
	{
		static int s_evType = 1;
		return s_evType++;
	}

	EventDispatcher(void);
	virtual ~EventDispatcher(void);

	void addEventListener(int evType, IEventListener* listener);
	void removeEventListener(int evType, IEventListener* listener);
	void removeEventListeners(int evType);
	bool hasEventListenerForType(int evType) const;
	void dispatchEvent(int evType, void* evData = NULL);
private:
	enum {
		kIterList = 0,
		kAddList = 1,
		kRemoveList = 2,
		kNumEvTypeListeners = 3
	};

	void invalidate(int evType);
	int getAddListIndex(int evType);
	int getRemoveListIndex(int evType);

	bool mInvalidated;
	int mDispatchType;
	std::map<int, std::set<IEventListener*>*>* mEventListeners;
};
#endif // __EVENT_DISPATCHER_H__
