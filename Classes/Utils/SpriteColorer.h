#ifndef __SPRITE_COLORER_H__
#define __SPRITE_COLORER_H__

#include "cocos2d.h"
#include <Interfaces/IAnimatable.h>
#include <Events/EventDispatcher.h>
#include <Utils/ByteTweener.h>
class IEventListener;
USING_NS_CC;

class SpriteColorer : public CCObject, public IAnimatable
{
public:
	static int EV_TYPE_COLOR_DID_CHANGE() { static int evCompleted = EventDispatcher::nextEvType(); return evCompleted; }

	SpriteColorer(CCSprite* sprite, IEventListener* listener = NULL);
	virtual ~SpriteColorer(void);

	void animateColor(uint color, float duration, float delay = 0.0f);
	void animateColor(const ccColor3B& color, float duration, float delay = 0.0f);
	virtual void advanceTime(float dt);

	virtual bool isComplete(void) const { return mIsComplete; }
	const ccColor3B getColor(void) const;
	void setColor(const ccColor3B& value);
	void setColor(uint value);
	IEventListener* getListener(void) { return mListener; }
	void setListener(IEventListener* value) { mListener = value; }

private:
	static const int kRedIndex = 0;
    static const int kGreenIndex = 1;
    static const int kBlueIndex = 2;
	void updateColor(const ccColor3B& color);
	void setComplete(bool value) { mIsComplete = value; }

	bool mIsComplete;
	GLubyte mColorParts[3];
    ByteTweener mTweeners[3];
	CCSprite* mSprite;
	IEventListener *mListener; // Callback state change notifier
};
#endif // __SPRITE_COLORER_H__
