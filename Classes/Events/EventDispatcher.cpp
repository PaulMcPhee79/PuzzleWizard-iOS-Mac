
#include "EventDispatcher.h"
USING_NS_CC;

typedef std::set<IEventListener*> EvList;
typedef std::map<int, EvList*> EvMap;

EventDispatcher::EventDispatcher(void)
	: mEventListeners(NULL), mInvalidated(false), mDispatchType(-1)
{

}

EventDispatcher::~EventDispatcher(void)
{
	if (mEventListeners)
	{
		for(EvMap::iterator it = mEventListeners->begin(); it != mEventListeners->end(); ++it)
			delete[] it->second;
		delete mEventListeners;
		mEventListeners = NULL;
	}	
}

void EventDispatcher::invalidate(int evType)
{
	mInvalidated = evType == mDispatchType;
}

int EventDispatcher::getAddListIndex(int evType)
{
	return evType == mDispatchType ? kAddList : kIterList;
}

int EventDispatcher::getRemoveListIndex(int evType)
{
	return evType == mDispatchType ? kRemoveList : kIterList;
}

void EventDispatcher::addEventListener(int evType, IEventListener* listener)
{
	if (listener == NULL)
		return;

	if (mEventListeners == NULL)
		mEventListeners = new EvMap();

	EvList* listeners = (*mEventListeners)[evType];
	if (!listeners)
	{
		listeners = new EvList[3];
		(*mEventListeners)[evType] = listeners;
	}

	listeners[getAddListIndex(evType)].insert(listener);
}

void EventDispatcher::removeEventListener(int evType, IEventListener* listener)
{
	if (listener == NULL || mEventListeners == NULL)
		return;

	EvList* listeners = (*mEventListeners)[evType];
	if (listeners)
	{
		if (getRemoveListIndex(evType) == kIterList)
			listeners[kIterList].erase(listener);
		else
			listeners[kRemoveList].insert(listener);
	}
}

void EventDispatcher::removeEventListeners(int evType)
{
	if (mEventListeners == NULL)
		return;

	EvList* listeners = (*mEventListeners)[evType];
	if (listeners)
	{
		for (int i = 0; i < kNumEvTypeListeners; ++i)
			listeners[i].clear();
		invalidate(evType);
		// Leave the empty listener array in the dictionary
	}
}

bool EventDispatcher::hasEventListenerForType(int evType) const
{
	if (mEventListeners)
	{
		EvMap::iterator it = mEventListeners->find(evType);
		return it != mEventListeners->end() && it->second[kIterList].size() > 0;
	}
	else
		return false;
}

void EventDispatcher::dispatchEvent(int evType, void* evData)
{
	if (mEventListeners == NULL)
		return;

	EvList* listeners = (*mEventListeners)[evType];
	if (listeners)
	{
		mInvalidated = false;
		mDispatchType = evType;
		for(EvList::iterator it = listeners[kIterList].begin(); it != listeners[kIterList].end(); ++it)
		{
			IEventListener* listener = *it;
			listener->onEvent(evType, evData);

			if (mInvalidated)
				break;
		}

		mDispatchType = -1;
		for(EvList::iterator it = listeners[kAddList].begin(); it != listeners[kAddList].end(); ++it)
			listeners[kIterList].insert(*it);
		for(EvList::iterator it = listeners[kRemoveList].begin(); it != listeners[kRemoveList].end(); ++it)
			listeners[kIterList].erase(*it);
		listeners[kAddList].clear();
		listeners[kRemoveList].clear();
	}
}

