#ifndef __INAVIGABLE_H__
#define __INAVIGABLE_H__

#include "cocos2d.h"

enum NavigationMap
{
    NAV_NONE,
    NAV_VERT,
    NAV_HORIZ,
    NAV_OMNI
};

class INavigable
{
public:
    INavigable(NavigationMap navMap) : mNavMap(navMap) { }
	virtual ~INavigable(void) { }
    
    virtual NavigationMap getNavMap(void) const { return mNavMap; }
    virtual void setNavMap(NavigationMap value) { mNavMap = value; }
    virtual cocos2d::CCNode* getCurrentNav(void) const = 0;
    virtual void resetNav(void) = 0;
    virtual void movePrevNav(void) = 0;
    virtual void moveNextNav(void) = 0;
    
protected:
    NavigationMap mNavMap;
};
#endif // __INAVIGABLE_H__
