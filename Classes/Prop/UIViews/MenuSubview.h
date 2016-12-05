#ifndef __MENU_SUBVIEW_H__
#define __MENU_SUBVIEW_H__

#include "cocos2d.h"
#include <Prop/Prop.h>
USING_NS_CC;

class MenuSubview : public Prop
{
public:
	MenuSubview(void);
	virtual ~MenuSubview(void);

	bool shouldDestroy(void) const { return mShouldDestroy; }
	void setShouldDestroy(bool value) { mShouldDestroy = value; }

private:
	bool mShouldDestroy;
};
#endif // __MENU_SUBVIEW_H__
