
#include "TouchPad.h"
USING_NS_CC;

TouchPad::TouchPad(const CCRect& touchBounds, IEventListener* listener)
	:
	mTouchBounds(touchBounds),
	mTouchEnabled(true),
	mMultiTouch(false),
	mTouchID(-1),
	mTouchPos(ccp(0, 0)),
	mPrevTouchPos(ccp(0, 0)),
	mListener(listener)
{
	
}

TouchPad::~TouchPad(void)
{
	mListener = NULL;
}

bool TouchPad::containsTouchLocation(CCTouch* pTouch, CCPoint& convertedPoint)
{
	convertedPoint = convertTouchToNodeSpace(pTouch);
    return mTouchBounds.containsPoint(convertedPoint);
}

void TouchPad::onEnter()
{
    CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, 0, true);
    CCNode::onEnter();
}

void TouchPad::onExit()
{
    CCDirector::sharedDirector()->getTouchDispatcher()->removeDelegate(this);
    CCNode::onExit();
}

void TouchPad::resetTouches(void)
{
	mTouchID = -1; // See CCEGLViewProtocol::getUnUsedIndex()
}

bool TouchPad::shouldReportTouch(CCTouch *pTouch)
{
	return pTouch && isTouchEnabled() && (mMultiTouch || (mTouchID == -1 || mTouchID == pTouch->getID()));
}

void TouchPad::reportTouch(int evType)
{
	if (getListener())
		getListener()->onEvent(evType, this);
}

bool TouchPad::ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent)
{
	//CCLog("++++++ Touch Began +++++++");
	if (shouldReportTouch(pTouch))
	{
		mTouchID = pTouch->getID();

		CCPoint convertedPoint;
		if (containsTouchLocation(pTouch, convertedPoint))
		{
			//CCLog("Touch Began at X: %f Y: %f\n", convertedPoint.x, convertedPoint.y);
			mTouchPos = convertedPoint;
			mPrevTouchPos = convertedPoint;
			reportTouch(EV_TYPE_TOUCH_BEGAN());
			return true;
		}
	}
	
	return false;
}

void TouchPad::ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent)
{
	if (shouldReportTouch(pTouch))
	{
		CCPoint convertedPoint;
		if (containsTouchLocation(pTouch, convertedPoint))
		{
			mPrevTouchPos = mTouchPos;
			mTouchPos = convertedPoint;
			reportTouch(EV_TYPE_TOUCH_MOVED());
		}
	}
}

void TouchPad::ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent)
{
	if (shouldReportTouch(pTouch))
	{
		CCPoint convertedPoint;
		if (containsTouchLocation(pTouch, convertedPoint))
		{
			mPrevTouchPos = mTouchPos;
			mTouchPos = convertedPoint;
			reportTouch(EV_TYPE_TOUCH_ENDED());
		}

		resetTouches();
	}
}

void TouchPad::ccTouchCancelled(CCTouch *pTouch, CCEvent *pEvent)
{
	if (shouldReportTouch(pTouch))
	{
		mPrevTouchPos = mTouchPos;
		reportTouch(EV_TYPE_TOUCH_CANCELLED());
		resetTouches();
	}
}
