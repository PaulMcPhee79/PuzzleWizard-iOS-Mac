
#define CM_KVSTORE_ONLY

#import "CloudManager.h"

// Local
#define LOCAL_KEY_UBIQ_ID_TOKENS @"UbiquityIdTokens"
#define LOCAL_KEY_UBIQ_ID_TOKEN_PREFIX @"UbiquityIdToken_"
#define LOCAL_KEY_LAST_KNOWN_UBIQ @"LastKnownUbiq"
#define LOCAL_KEY_SAVE_GAME @"LocalPuzzleWizardSaveKey"

// Cloud
#define CLOUD_KEY_SAVE_GAME @"iCloudPuzzleWizardSaveKey"
#define CLOUD_KEY_SETTINGS @"iCloudPuzzleWizardSettingsKey"


@interface CloudManager ()

@property (nonatomic, copy) id<NSObject, NSCopying, NSCoding> ubiquityIdToken;
@property (nonatomic, copy) id<NSObject, NSCopying, NSCoding> prevUbiquityIdToken;
@property (nonatomic, readonly) id<NSObject, NSCopying, NSCoding> localUbiquityIdToken;

- (void)registerForNotifications;
- (void)unregisterForNotifications;
- (void)cloudInitSucceeded;
- (void)cloudInitFailed;

- (NSString *)getUnusedUbiquityIdTokenKey;
- (id<NSObject, NSCopying, NSCoding>)getUbiquityIdTokenForKey:(NSString *)key;
- (NSString *)getUbiquityIdTokenKey:(id<NSObject, NSCopying, NSCoding>)token;
- (id<NSObject, NSCopying, NSCoding>)pollActiveUbiquityIdToken;
- (void)addUbiquityIdToken:(id<NSObject, NSCopying, NSCoding>)ubiqIdToken;

- (NSData *)getDataForKey:(NSString *)key;
- (void)setData:(NSData *)data forKey:(NSString *)key;

- (NSData *)getCloudDataForKey:(NSString *)key;
- (void)setCloudData:(NSData *)data forKey:(NSString *)key;
- (void)iCloudDataDidChangeExternally:(NSNotification *)notification;
- (void)iCloudAccountDidChange:(NSNotification *)notification;

@end


@implementation CloudManager

static CloudManager *_CM = nil;

+ (CloudManager *)CM {
	@synchronized(self) {
		if (_CM == nil) {
			_CM = [[self alloc] init];
		}
	}
	return _CM;
}

+ (id)allocWithZone:(NSZone *)zone {
	@synchronized(self) {
		if (_CM == nil) {
			_CM = [super allocWithZone:zone];
			return _CM;
		}
	}
	
	return nil;
}

- (id)copyWithZone:(NSZone *)zone {
	return self;
}

- (id)retain {
	return self;
}

- (NSUInteger)retainCount {
	return NSUIntegerMax;
}

- (oneway void)release {
	
}

- (id)autorelease {
	return self;
}

// ------------ End singleton junk -------------

@synthesize isCloudApproved = mCloudApproved;
@synthesize ubiquityIdToken = mUbiqIdToken;
@synthesize prevUbiquityIdToken = mPrevUbiqIdToken;
@synthesize ubiqIdSupported = mUbiqIdSupported;

+ (int)CUST_EVENT_TYPE_CLOUD_LOGGED_IN
{
    static int evCompleted = EventDispatcher::nextEvType();
    return evCompleted;
}

+ (int)CUST_EVENT_TYPE_CLOUD_LOGGED_OUT
{
    static int evCompleted = EventDispatcher::nextEvType();
    return evCompleted;
}

+ (int)CUST_EVENT_TYPE_CLOUD_DATA_CHANGED
{
    static int evCompleted = EventDispatcher::nextEvType();
    return evCompleted;
}

+ (int)CUST_EVENT_TYPE_CLOUD_SETTINGS_CHANGED
{
    static int evCompleted = EventDispatcher::nextEvType();
    return evCompleted;
}

+ (int)CUST_EVENT_TYPE_CLOUD_UNAVAILABLE
{
    static int evCompleted = EventDispatcher::nextEvType();
    return evCompleted;
}

- (id)init {
    if (self = [super init]) {
        mCloudApproved = mCloudEnabled = mBusyEnabling = mLocalSaveInProgress = NO;
        mUbiqIdSupported = [[NSFileManager defaultManager] respondsToSelector:@selector(ubiquityIdentityToken)];
        mUbiqIdToken = nil;
        mPrevUbiqIdToken = nil;
    }
    return self;
}

- (void)dealloc {
    [self unregisterForNotifications];
    [super dealloc];
}

- (BOOL)isCloudSupported {
    return NSClassFromString(@"NSUbiquitousKeyValueStore") != nil;
}

- (BOOL)hasActiveCloudAccount {
#ifdef CM_KVSTORE_ONLY
    return [self isCloudSupported];
#else
    return (mCloudEnabled && (!mUbiqIdSupported || self.ubiquityIdToken != nil));
#endif
}

- (id<NSObject, NSCopying, NSCoding>)localUbiquityIdToken {
    return self.ubiquityIdToken ? self.ubiquityIdToken : self.prevUbiquityIdToken;
}

- (void)enableCloud {
    NSLog(@"iCloud enableCloud called.");
    
    if (!self.isCloudApproved || mCloudEnabled || mBusyEnabling)
        return;
    
    if (NSClassFromString(@"NSUbiquitousKeyValueStore")) { // is iOS 5?
        NSLog(@"iCloud supported.");
        mBusyEnabling = YES;
        
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^(void) {
            if ([NSUbiquitousKeyValueStore defaultStore]) { // && [[NSFileManager defaultManager] URLForUbiquityContainerIdentifier:nil] != nil) {
                dispatch_async(dispatch_get_main_queue(), ^(void) {
                    [self cloudInitSucceeded];
                });
            } else {
                dispatch_async(dispatch_get_main_queue(), ^(void) {
                    [self cloudInitFailed];
                });
            }
        });
    } else {
        NSLog(@"iCloud not supported.");
    }
}

- (void)cloudInitSucceeded {
    mBusyEnabling = NO;
    
    [self registerForNotifications];
    self.ubiquityIdToken = [self pollActiveUbiquityIdToken];
    [self addUbiquityIdToken:self.ubiquityIdToken];
    mCloudEnabled = YES;
    
    if (self.hasActiveCloudAccount) {
        NSLog(@"iCloud initialized successfully.");
        if ([[NSUbiquitousKeyValueStore defaultStore] synchronize])
            NSLog(@"iCloud initial synchronize succeeded.");
        else
            NSLog(@"iCloud initial synchronize failed.");
        mEvDispatcher.dispatchEvent([CloudManager CUST_EVENT_TYPE_CLOUD_LOGGED_IN]);
    }
    else {
        NSLog(@"No active iCloud account found.");
        mEvDispatcher.dispatchEvent([CloudManager CUST_EVENT_TYPE_CLOUD_UNAVAILABLE]);
    }
}

- (void)cloudInitFailed {
    mBusyEnabling = NO;
    NSLog(@"iCloud failed to initialize.");
    mEvDispatcher.dispatchEvent([CloudManager CUST_EVENT_TYPE_CLOUD_UNAVAILABLE]);
}

- (void)disableCloud {
    mCloudEnabled = NO;
    [self unregisterForNotifications];
}

- (NSData *)load {
    if (self.hasActiveCloudAccount)
        return [self getCloudDataForKey:CLOUD_KEY_SAVE_GAME];
    else
        return nil;
}

- (void)save:(NSData *)data {
    if (self.hasActiveCloudAccount && data)
        [self setCloudData:data forKey:CLOUD_KEY_SAVE_GAME];
}

- (void)registerForNotifications {
    [self unregisterForNotifications];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(iCloudDataDidChangeExternally:)
                                                 name:NSUbiquitousKeyValueStoreDidChangeExternallyNotification
                                               object:[NSUbiquitousKeyValueStore defaultStore]];
    if (self.ubiqIdSupported)
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(iCloudAccountDidChange:)
                                                     name:NSUbiquityIdentityDidChangeNotification
                                                   object:nil];
}

- (void)unregisterForNotifications {
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:NSUbiquitousKeyValueStoreDidChangeExternallyNotification
                                                  object:[NSUbiquitousKeyValueStore defaultStore]];
    if (self.ubiqIdSupported)
        [[NSNotificationCenter defaultCenter] removeObserver:self
                                                        name:NSUbiquityIdentityDidChangeNotification
                                                      object:nil];
}

- (void)applicationDidBecomeActive {
    if (!self.isCloudApproved)
        return;
    
    [self registerForNotifications];

#ifdef CM_KVSTORE_ONLY
    [[NSUbiquitousKeyValueStore defaultStore] synchronize];
#else
    if (self.ubiqIdSupported) {
        if (self.ubiquityIdToken && self.ubiquityIdToken == [self pollActiveUbiquityIdToken])
            [[NSUbiquitousKeyValueStore defaultStore] synchronize];
        else if (!mBusyEnabling) {
            [self disableCloud];
            [self enableCloud];
        }
    } else if (!mBusyEnabling) {
        [self disableCloud];
        [self enableCloud];
    }
#endif
}

- (void)applicationWillResignActive {
    if (mCloudEnabled)
        [self unregisterForNotifications];
}

- (void)addEventListener:(int)evType listener:(void*)listener {
    if (listener)
        mEvDispatcher.addEventListener(evType, (IEventListener *)listener);
}

- (void)removeEventListener:(int)evType listener:(void*)listener {
    if (listener)
        mEvDispatcher.removeEventListener(evType, (IEventListener *)listener);
}

- (void)removeEventListeners:(int)evType {
    mEvDispatcher.removeEventListeners(evType);
}

- (NSString *)getUnusedUbiquityIdTokenKey {
    NSDictionary *tokenDict = (NSDictionary *)[[NSUserDefaults standardUserDefaults] objectForKey:LOCAL_KEY_UBIQ_ID_TOKENS];
    return [NSString stringWithFormat:@"%@%d", LOCAL_KEY_UBIQ_ID_TOKEN_PREFIX, (tokenDict ? (int)tokenDict.count + 1 : 1)];
}

- (id<NSObject, NSCopying, NSCoding>)getUbiquityIdTokenForKey:(NSString *)key {
    if (key == nil)
        return nil;
    
    NSDictionary *tokenDict = (NSDictionary *)[[NSUserDefaults standardUserDefaults] objectForKey:LOCAL_KEY_UBIQ_ID_TOKENS];
    return tokenDict ? [tokenDict objectForKey:key] : nil;
}

- (NSString *)getUbiquityIdTokenKey:(id<NSObject, NSCopying, NSCoding>)token {
    if (token == nil)
        return nil;
    
    NSDictionary *tokenDict = (NSDictionary *)[[NSUserDefaults standardUserDefaults] objectForKey:LOCAL_KEY_UBIQ_ID_TOKENS];
    if (tokenDict) {
        for (NSString *key in tokenDict) {
            id<NSObject, NSCopying, NSCoding> otherToken = (id<NSObject, NSCopying, NSCoding>)[tokenDict objectForKey:key];
            if ([token isEqual:otherToken])
                return key;
        }
    }
    
    return nil;
}

- (id<NSObject, NSCopying, NSCoding>)pollActiveUbiquityIdToken {
    if (self.ubiqIdSupported) {
        id<NSObject, NSCopying, NSCoding> token = [[NSFileManager defaultManager] ubiquityIdentityToken];
        return token;
    } else
        return nil;
}

- (void)addUbiquityIdToken:(id<NSObject, NSCopying, NSCoding>)token {
    if (token) {
        NSDictionary *tokenDict = (NSDictionary *)[[NSUserDefaults standardUserDefaults] objectForKey:LOCAL_KEY_UBIQ_ID_TOKENS];
        if (tokenDict) {
            // Don't add this token twice
            for (NSString *key in tokenDict) {
                id<NSObject, NSCopying, NSCoding> otherToken = (id<NSObject, NSCopying, NSCoding>)[tokenDict objectForKey:key];
                if ([token isEqual:otherToken])
                    return;
            }
        }
        
        NSMutableDictionary *newTokenDict = tokenDict
            ? [NSMutableDictionary dictionaryWithDictionary:tokenDict]
            : [NSMutableDictionary dictionary];
        [newTokenDict setObject:token forKey:[self getUnusedUbiquityIdTokenKey]];

        [[NSUserDefaults standardUserDefaults] setObject:newTokenDict forKey:LOCAL_KEY_UBIQ_ID_TOKENS];
        [[NSUserDefaults standardUserDefaults] synchronize];
    }
}

- (NSData *)getDataForKey:(NSString *)key {
    NSData *data = nil;
    if (key)
        data = (NSData *)[[NSUserDefaults standardUserDefaults] objectForKey:key];
    return data;
}

- (void)setData:(NSData *)data forKey:(NSString *)key {
    if (data && key) {
        [[NSUserDefaults standardUserDefaults] setObject:[NSData dataWithData:data] forKey:key];
        [[NSUserDefaults standardUserDefaults] synchronize];
    }
}

- (NSData *)getCloudDataForKey:(NSString *)key {
    NSData *data = nil;
    if (key && self.hasActiveCloudAccount)
        data = (NSData *)[[NSUbiquitousKeyValueStore defaultStore] objectForKey:key];
    return data;
}

- (void)setCloudData:(NSData *)data forKey:(NSString *)key {
    if (data == nil || key == nil || !self.hasActiveCloudAccount || mLocalSaveInProgress)
        return;
    
    [[NSUbiquitousKeyValueStore defaultStore] setObject:[NSData dataWithData:data] forKey:key];
}

- (void)iCloudDataDidChangeExternally:(NSNotification *)notification {
    BOOL initialDownload = NO, storeServerChange = YES;
    NSDictionary *userInfo = notification.userInfo;
    if (userInfo && [userInfo objectForKey:NSUbiquitousKeyValueStoreChangeReasonKey]) {
        storeServerChange = NO;
        
        NSNumber *reason = (NSNumber *)[userInfo objectForKey:NSUbiquitousKeyValueStoreChangeReasonKey];
        switch ([reason integerValue]) {
            case NSUbiquitousKeyValueStoreInitialSyncChange:
                NSLog(@"Initial iCloud download.");
                initialDownload = YES;
                //NSLog(@"Attempt to write to iCloud key-value storage discarded because an initial download from iCloud has not yet happened.");
                break;
            case NSUbiquitousKeyValueStoreQuotaViolationChange:
                NSLog(@"iCloud key-value store has exceeded its space quota on the iCloud server.");
                break;
            case NSUbiquitousKeyValueStoreAccountChange:
                // Ignore: we listen for NSUbiquityIdentityDidChangeNotification for account changes,
                break;
            case NSUbiquitousKeyValueStoreServerChange:
            default:
                storeServerChange = YES;
                break;
        }
    }
    
    // If it's not an error, assume it's a value change. Docs say userInfo 'can' contain
    // change reason. So if it doesn't contain a reason, we'll assume a value change reason.
    if (initialDownload) {
        mEvDispatcher.dispatchEvent([CloudManager CUST_EVENT_TYPE_CLOUD_DATA_CHANGED]);
        //mEvDispatcher.dispatchEvent(CUST_EVENT_TYPE_CLOUD_SETTINGS_CHANGED());
    } else if (storeServerChange) {
        NSArray *changedKeys = (NSArray *)[userInfo objectForKey:NSUbiquitousKeyValueStoreChangedKeysKey];
        if (changedKeys && changedKeys.count > 0) {
            BOOL dispatchedData = NO, dispatchedSettings = NO;
            for (NSString *key in changedKeys) {
                if (dispatchedData && dispatchedSettings)
                    break;
                
                if ([key isEqualToString:CLOUD_KEY_SETTINGS]) {
                    if (!dispatchedSettings) {
                        dispatchedSettings = YES;
                        mEvDispatcher.dispatchEvent([CloudManager CUST_EVENT_TYPE_CLOUD_SETTINGS_CHANGED]);
                    }
                } else {
                    if (!dispatchedData) {
                        dispatchedData = YES;
                        mEvDispatcher.dispatchEvent([CloudManager CUST_EVENT_TYPE_CLOUD_DATA_CHANGED]);
                    }
                }
            }
        }
        else {
            mEvDispatcher.dispatchEvent([CloudManager CUST_EVENT_TYPE_CLOUD_DATA_CHANGED]);
            //mEvDispatcher.dispatchEvent(CUST_EVENT_TYPE_CLOUD_SETTINGS_CHANGED());
        }
    }
}

- (void)iCloudAccountDidChange:(NSNotification *)notification {
#ifdef CM_KVSTORE_ONLY
    [[NSUbiquitousKeyValueStore defaultStore] synchronize];
    mEvDispatcher.dispatchEvent([CloudManager CUST_EVENT_TYPE_CLOUD_LOGGED_IN]);
#else
    id<NSObject, NSCopying, NSCoding> polledToken = [self pollActiveUbiquityIdToken];
    if (self.ubiquityIdToken == polledToken)
        return;
    
    if (self.ubiquityIdToken) {
        self.prevUbiquityIdToken = self.ubiquityIdToken;
        self.ubiquityIdToken = nil;
        mEvDispatcher.dispatchEvent([CloudManager CUST_EVENT_TYPE_CLOUD_LOGGED_OUT]);
    }
    
    self.ubiquityIdToken = polledToken;
    [self addUbiquityIdToken:self.ubiquityIdToken];
    
    if (self.ubiquityIdToken && self.ubiquityIdToken != self.prevUbiquityIdToken) {
        [[NSUbiquitousKeyValueStore defaultStore] synchronize];
        mEvDispatcher.dispatchEvent([CloudManager CUST_EVENT_TYPE_CLOUD_LOGGED_IN]);
    }
#endif
}

@end
