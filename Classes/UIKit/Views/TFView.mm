
#import "TFView.h"

@interface TFView ()

- (void)onSubmitPressed:(id)sender;

@end


@implementation TFView

@synthesize onSubmit = mOnSubmit;

- (id)initWithFrame:(CGRect)aRect
{
	if (self = [super initWithFrame:aRect])
    {
        [self setBackgroundColor:[UIColor whiteColor]];
	}
	return self;
}

- (void)onSubmitPressed:(id)sender
{
    if (mOnSubmit)
        mOnSubmit(sender);
}

- (void)setTextFieldDelegate:(id<UITextFieldDelegate>)tfDelegate { /* override me */ }

- (void)setTextViewDelegate:(id<UITextViewDelegate>)tvDelegate { /* override me */ }

@end
