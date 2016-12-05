
#import "AlertViewNS.h"
#import "IEventListener.h"

@interface AlertViewNS ()

- (void)alertDidEnd:(NSAlert *)alert returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo;

@end


@implementation AlertViewNS

@synthesize tag = mTag;

- (id)initWithListener:(IEventListener *)listener cancelButton:(BOOL)cancelButton
{
    if (self = [super init])
    {
        mListener = listener;
        mIsShowing = mDidShow = NO;
        mTag = 0;
        mAlert = [[NSAlert alloc] init];
        [mAlert setAlertStyle:NSInformationalAlertStyle];
        if (cancelButton)
            [mAlert addButtonWithTitle:NSLocalizedString(@"Cancel", nil)];
    }
    
    return self;
}

- (void)setAlertStyle:(NSAlertStyle)style
{
    [mAlert setAlertStyle:style];
}

- (void)setTitle:(NSString *)titleKey
{
    if (titleKey)
    {
        NSString* title = NSLocalizedString(titleKey, nil);
        if (title)
            [mAlert setMessageText:title];
    }
}

- (void)setMessage:(NSString *)messageKey
{
    if (messageKey)
    {
        NSString* message = NSLocalizedString(messageKey, nil);
        if (message)
            [mAlert setInformativeText:message];
    }
}

- (void)addButtonWithTitle:(NSString *)titleKey
{
    if (titleKey)
    {
        NSString* title = NSLocalizedString(titleKey, nil);
        if (title)
            [mAlert addButtonWithTitle:title];
    }
}

- (void)show
{
    if (mDidShow == NO && mIsShowing == NO)
    {
        mIsShowing = YES;
        [self retain];
        //int response = (int)[mAlert runModal];
        //if (mListener)
        //    mListener->onEvent((int)self.tag, &response);
        //[self autorelease];
        
        [mAlert beginSheetModalForWindow:[NSApplication sharedApplication].keyWindow
                           modalDelegate:self
                          didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:)
                             contextInfo:nil];
    }
}

- (void)alertDidEnd:(NSAlert *)alert returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    if (mDidShow)
        return;
    
    mDidShow = true;
    mIsShowing = false;
    
    if (mListener)
    {
        int response = (int)returnCode;
        mListener->onEvent((int)self.tag, &response);
    }
    
    [self autorelease];
}

- (void)dealloc
{
    [mAlert release], mAlert = nil;
    mListener = NULL;
    [super dealloc];
}

@end
