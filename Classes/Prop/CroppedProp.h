#ifndef __CROPPED_PROP_H__
#define __CROPPED_PROP_H__

#include "cocos2d.h"
#include "Prop.h"

class CroppedProp : public Prop
{
public:
	CroppedProp(int category = -1, const CCRect& viewableRegion = CCRectZero);
	virtual ~CroppedProp(void);

	CCRect getViewableRegion();
	void setViewableRegion(const CCRect& value);
	void clampToContent();
    void enableCrop(bool enable) { mEnabled = enable; }

	virtual void visit(void);

protected:
	void verifyViewableRegion();

private:
    bool mEnabled;
	CCRect mViewableRegion;
};
#endif // __CROPPED_PROP_H__
