#ifndef __UTILS_NS_H__
#define __UTILS_NS_H__

#include "cocos2d.h"

class UtilsNS
{
public:
    static void CMLog(const char* msg);
    
private:
    UtilsNS();
    ~UtilsNS(void);
    UtilsNS(const UtilsNS& other);
	UtilsNS& operator=(const UtilsNS& rhs);
};
#endif // __UTILS_NS_H__
