#ifndef __PUZZLE_RIBBON_H__
#define __PUZZLE_RIBBON_H__

#include "cocos2d.h"
#include <Prop/Prop.h>
#include <Interfaces/IEventListener.h>
#include <Interfaces/ILocalizable.h>
class ByteTweener;
class FloatTweener;
USING_NS_CC;

class PuzzleRibbon : public Prop, public IEventListener, public ILocalizable
{
public:
	PuzzleRibbon(int category);
	virtual ~PuzzleRibbon(void);

	virtual bool init(void);
    
    virtual void localeDidChange(const char* fontKey, const char* FXFontKey);

	void animate(float duration);
	void stopAnimating(void);

	void setLevelString(const char* value);
	void setLevelColor(uint value);
	void setPuzzleString(const char* value);
	void setPuzzleColor(uint value);

    void resolutionDidChange(void);
	virtual void onEvent(int evType, void* evData);
	virtual void advanceTime(float dt);
private:
	float mDuration;

	CCLabelBMFont* mLevelLabel;
	CCLabelBMFont* mPuzzleLabel;
    CCSprite* mUpperStripe;
    CCSprite* mLowerStripe;
	CCSprite* mBgQuad;

	ByteTweener* mOpacityTweener;
	FloatTweener* mLevelLabelTweener;
	FloatTweener* mPuzzleLabelTweener;
};
#endif // __PUZZLE_RIBBON_H__
