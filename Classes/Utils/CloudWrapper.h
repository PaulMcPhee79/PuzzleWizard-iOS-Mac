#ifndef __CLOUD_WRAPPER_H__
#define __CLOUD_WRAPPER_H__

#include "cocos2d.h"
USING_NS_CC;

class IEventListener;

class CloudWrapper
{
public:
    static bool isCloudApproved(void);
    static void setCloudApproved(bool value);
    
    static bool isCloudSupported(void);
    static bool hasActiveCloudAccount(void);
    
    static void enableCloud(bool enable);
    
    static const void* load(ulong& size);
    static void save(const void* data, ulong size);
    
    static void applicationDidBecomeActive(void);
    static void applicationWillResignActive(void);
    
    static void addEventListener(int evType, IEventListener* listener);
	static void removeEventListener(int evType, IEventListener* listener);
	static void removeEventListeners(int evType);
    
    static int getEvTypeLoggedIn(void);
    static int getEvTypeLoggedOut(void);
    static int getEvTypeDataChanged(void);
    static int getEvTypeSettingsChanged(void);
    static int getEvTypeUnavailable(void);
    
private:
    CloudWrapper();
    ~CloudWrapper(void);
    CloudWrapper(const CloudWrapper& other);
	CloudWrapper& operator=(const CloudWrapper& rhs);
};
#endif // __CLOUD_WRAPPER_H__
