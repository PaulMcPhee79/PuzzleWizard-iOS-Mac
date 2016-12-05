
#import "AlertView.h"
#import "IEventListener.h"

@interface AlertView ()


@end


@implementation AlertView

@synthesize tag = mTag;

- (id)initWithListener:(IEventListener *)listener cancelButton:(BOOL)cancelButton
{
    if (self = [super init])
    {
        mListener = listener;
        mIsShowing = mDidShow = NO;
        mTag = 0;
        mAlertView = [[UIAlertView alloc] initWithTitle:@""
                                                message:@""
                                               delegate:self
                                      cancelButtonTitle:(cancelButton ? NSLocalizedString(@"Cancel", nil) : nil)
                                      otherButtonTitles:nil];
    }
    
    return self;
}

- (void)setTitle:(NSString *)titleKey
{
    if (titleKey)
    {
        NSString* title = NSLocalizedString(titleKey, nil);
        if (title)
            mAlertView.title = title;
    }
}

- (void)setMessage:(NSString *)messageKey
{
    if (messageKey)
    {
        NSString* message = NSLocalizedString(messageKey, nil);
        if (message)
            mAlertView.message = message;
    }
}

- (void)addButtonWithTitle:(NSString *)titleKey
{
    if (titleKey)
    {
        NSString* title = NSLocalizedString(titleKey, nil);
        if (title)
            [mAlertView addButtonWithTitle:title];
    }
}

- (void)show
{
    if (mDidShow == NO && mIsShowing == NO)
    {
        mIsShowing = YES;
        [self retain];
        [mAlertView show];
    }
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
    if (mListener)
    {
        int param = (int)buttonIndex;
        mListener->onEvent((int)self.tag, &param);
    }
}

- (void)alertViewCancel:(UIAlertView *)alertView
{
    if (mListener)
    {
        int param = -1;
        mListener->onEvent((int)self.tag, &param);
    }
}

- (void)alertView:(UIAlertView *)alertView willDismissWithButtonIndex:(NSInteger)buttonIndex
{
    if (mDidShow == NO && mIsShowing == YES)
    {
        mIsShowing = NO;
        mDidShow = YES;
        [self autorelease];
    }
}

- (void)dealloc
{
    [mAlertView release], mAlertView = nil;
    mListener = NULL;
    [super dealloc];
}

@end
