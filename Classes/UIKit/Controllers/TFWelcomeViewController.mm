
#import "TFWelcomeViewController.h"
#import "TFWelcomeView.h"
#import "SceneController.h"
#import "GameSettings.h"
#include <Utils/BridgingUtility.h>

@interface TFWelcomeViewController ()

@property (nonatomic,readonly) TFWelcomeView* welcomeView;

@end

@implementation TFWelcomeViewController

- (void)loadView {
	[super loadView];
    
    CGRect screenRect = [UIScreen mainScreen].applicationFrame;
    TFWelcomeView* view = [[[TFWelcomeView alloc] initWithFrame:CGRectMake(0, 0, screenRect.size.height, screenRect.size.width)] autorelease];
    [view setTextFieldDelegate:self];
    view.onSubmit = ^(id sender)
    {
        NSString* username = view.username;
        if (username == nil)
            username = @"";
        else
        {
            SceneController::setUserDefault(CMSettings::S_BETA_NAME, [username UTF8String]);
            SceneController::flushUserDefaults();
            BridgingUtility::setUserIdentifier([username UTF8String]);
        }
        [self dismissViewControllerAnimated:YES completion:nil];
    };
    
    self.view = view;
}

- (TFWelcomeView *)welcomeView
{
    return self.view.subviews.count > 0 ? (TFWelcomeView*)self.view.subviews[0] : nil;
}

@end

