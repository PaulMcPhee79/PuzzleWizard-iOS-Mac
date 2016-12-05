#ifndef __PROGRESS_BAR_H__
#define __PROGRESS_BAR_H__

#include "cocos2d.h"
#include <Prop/Prop.h>
USING_NS_CC;

class ProgressBar : public Prop
{
public:
	ProgressBar(int category);
	virtual ~ProgressBar(void);
    
	static ProgressBar* create(int category, float width, float height, bool autorelease = true);
	virtual bool initWith(float width, float height);
    
    void setTrackColor(uint color);
	void setProgressColor(uint color);
    void setProgress(float value); // 0.0f - 1.0f
    void reconfigure(float width, float height);
    
private:
    float mProgressPercentage;
    CCSprite* mTrackBar;
    CCSprite* mProgressBar;
};
#endif // __PROGRESS_BAR_H__
