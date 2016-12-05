
#import "TFThanksViewController.h"
#import "TFThanksView.h"

@implementation TFThanksViewController

- (void)loadView {
	[super loadView];
    
    CGRect screenRect = [UIScreen mainScreen].applicationFrame;
    TFThanksView* view = [[[TFThanksView alloc] initWithFrame:CGRectMake(0, 0, screenRect.size.height, screenRect.size.width)] autorelease];
    view.onSubmit = ^(id sender)
    {
        [self dismissViewControllerAnimated:YES completion:nil];
    };
    
    self.view = view;
}

@end

