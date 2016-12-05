
#include "TouchManager.h"
#include <Puzzle/Inputs/TouchPad.h>
#include <algorithm>
USING_NS_CC;
using namespace CMTouches;

typedef std::vector<IEventListener*> TouchList;
typedef std::map<int, IEventListener*> TouchMap;

TouchManager::TouchManager(int category, const CCRect& touchBounds)
	:
Prop(category),
mEnabled(true),
mIsLocked(false),
mTimeSinceLastTouch(0),
mTouchPad(NULL)
{
	mTouchPad = new TouchPad(touchBounds, this);
	addChild(mTouchPad);

	mTouchListeners.resize(TC_COUNT, NULL);
	resetTouches();
}

TouchManager::~TouchManager(void)
{
	CC_SAFE_RELEASE_NULL(mTouchPad);
}

void TouchManager::setTouchBounds(const CCRect& value)
{
    mTouchPad->setTouchBounds(value);
}

void TouchManager::dispatchTouchNotices(int evType)
{
	mIsLocked = true;
    // Walk through backwards to enforce display list priority.
	for (int i = (int)mTouchListeners.size() - 1; i >= 0; --i)
	{
		IEventListener* listener = mTouchListeners[i];
		// Skip elements that have been removed during the iteration.
		if (listener == NULL || (mTouchNotice.focus && listener != mTouchNotice.focus) || (mRemoveQueue.size() > 0 && mRemoveQueue.count(i) != 0))
			continue;
		
		listener->onEvent(evType, &mTouchNotice);
		
		if (mTouchNotice.hasFocus())
			break;
	}
	mIsLocked = false;

	for(TouchMap::iterator it = mAddQueue.begin(); it != mAddQueue.end(); ++it)
		mTouchListeners[it->first] = it->second;
	for(TouchMap::iterator it = mRemoveQueue.begin(); it != mRemoveQueue.end(); ++it)
		mTouchListeners[it->first] = NULL;

	mAddQueue.clear();
	mRemoveQueue.clear();
}

void TouchManager::registerForTouches(TouchCategory category, IEventListener* listener)
{
#if DEBUG
    std::vector<IEventListener*>::iterator findIt = std::find(mTouchListeners.begin(), mTouchListeners.end(), listener);
    CCAssert(listener
             && category >= 0 && category < (int)mTouchListeners.size()
             && findIt == mTouchListeners.end()
             && mAddQueue.count(category) == 0, "TouchManager::registerForTouches - invalid args.");
#endif

	if (mIsLocked)
	{
		mRemoveQueue.erase(category);
		mAddQueue[category] = listener;
	}
	else
	{
		mTouchListeners[category] = listener;
	}
}

void TouchManager::deregisterForTouches(TouchCategory category, IEventListener* listener)
{
    CCAssert(listener && category >= 0 && category < (int)mTouchListeners.size(),
             "TouchManager::deregisterForTouches - invalid args.");
    
	if (listener == NULL || category < 0 || category >= (int)mTouchListeners.size())
		return;

	if (mIsLocked)
	{
		TouchList::iterator it = std::find(mTouchListeners.begin(), mTouchListeners.end(), listener);
		if (it != mTouchListeners.end() && (*it) == listener)
		{
			mAddQueue.erase(category);
			mRemoveQueue[category] = listener;
		}
	}
	else
	{
        mTouchListeners[(int)category] = NULL;
//		for (int i = 0; i < (int)mTouchListeners.size(); ++i)
//		{
//			if (mTouchListeners[i] == listener)
//				mTouchListeners[i] = NULL;
//		}
	}
}

void TouchManager::resetTouches(void)
{
	if (mTouchPad)
		mTouchPad->resetTouches();
}

void TouchManager::onEvent(int evType, void* evData)
{
    if (!isEnabled())
        return;
    
	CCAssert(!mIsLocked, "TouchManager::onEvent - invalid internal state.");

	mTouchNotice.pos = mTouchPad->getTouchPos();
	mTouchNotice.prevPos = mTouchPad->getPrevTouchPos();

	if (evType == TouchPad::EV_TYPE_TOUCH_BEGAN())
		mTouchNotice.reset();

    setTimeSinceLastTouch(0);
	dispatchTouchNotices(evType);
}
