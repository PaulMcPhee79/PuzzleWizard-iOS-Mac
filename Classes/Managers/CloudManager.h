
#ifndef __CLOUD_MANAGER_H__
#define __CLOUD_MANAGER_H__

#import <Foundation/Foundation.h>
#include "cocos2d.h"
#include <Events/EventDispatcher.h>
USING_NS_CC;

@interface CloudManager : NSObject {
    @private
    BOOL mCloudApproved;
    BOOL mCloudEnabled;
    BOOL mBusyEnabling;
    BOOL mUbiqIdSupported;
    BOOL mLocalSaveInProgress;
    id<NSObject, NSCopying, NSCoding> mUbiqIdToken;
    id<NSObject, NSCopying, NSCoding> mPrevUbiqIdToken;
    EventDispatcher mEvDispatcher;
}

@property (nonatomic, assign) BOOL isCloudApproved;
@property (nonatomic, readonly) BOOL isCloudSupported;
@property (nonatomic, readonly) BOOL ubiqIdSupported;
@property (nonatomic, readonly) BOOL hasActiveCloudAccount;

+ (CloudManager *)CM;

- (void)enableCloud;
- (void)disableCloud;

- (NSData *)load;
- (void)save:(NSData *)data;

- (void)applicationDidBecomeActive;
- (void)applicationWillResignActive;

- (void)addEventListener:(int)evType listener:(void*)listener;
- (void)removeEventListener:(int)evType listener:(void*)listener;
- (void)removeEventListeners:(int)evType;

+ (int)CUST_EVENT_TYPE_CLOUD_LOGGED_IN;
+ (int)CUST_EVENT_TYPE_CLOUD_LOGGED_OUT;
+ (int)CUST_EVENT_TYPE_CLOUD_DATA_CHANGED;
+ (int)CUST_EVENT_TYPE_CLOUD_SETTINGS_CHANGED;
+ (int)CUST_EVENT_TYPE_CLOUD_UNAVAILABLE;

@end

#endif // __CLOUD_MANAGER_H__
