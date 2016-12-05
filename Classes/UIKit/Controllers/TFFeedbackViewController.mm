
#import "TFFeedbackViewController.h"
#import "TFFeedbackView.h"
IMPORT_ANALYTICS

@interface TFFeedbackViewController ()

@property (nonatomic,readonly) TFFeedbackView* feedbackView;

@end

@implementation TFFeedbackViewController

- (void)loadView {
	[super loadView];
    
    CGRect screenRect = [UIScreen mainScreen].applicationFrame;
    TFFeedbackView* view = [[[TFFeedbackView alloc] initWithFrame:CGRectMake(0, 0, screenRect.size.height, 600.0f)] autorelease];
    [view setTextViewDelegate:self];
    view.onSubmit = ^(id sender)
    {
        NSString* questionLabelText = view.questionLabelText;
        NSString* questionUserText = view.questionUserText;
        NSString* feedbackLabelText = view.feedbackLabelText;
        NSString* feedbackUserText = view.feedbackUserText;
        if (questionLabelText == nil) questionLabelText = @"";
        if (questionUserText == nil) questionUserText = @"";
        if (feedbackLabelText == nil) feedbackLabelText = @"";
        if (feedbackUserText == nil) feedbackUserText = @"";
        
#if TF_BETA
        [TestFlight submitFeedback:
         [@[
            questionLabelText,
            @"\n", questionUserText,
            @"\n---------------------------",
            @"\n", feedbackLabelText,
            @"\n", feedbackUserText
            ] componentsJoinedByString:@""]];
#endif
        [self dismissViewControllerAnimated:YES completion:nil];
    };
    
    self.view = view;
}

- (TFFeedbackView *)feedbackView
{
    return self.view.subviews.count > 0 ? (TFFeedbackView*)self.view.subviews[0] : nil;
}

- (void)setHeaderText:(NSString *)text
{
    self.feedbackView.headerLabelText = text;
}

- (void)setQuestionText:(NSString *)text
{
    self.feedbackView.questionLabelText = text;
}

- (void)setFeedbackText:(NSString *)text
{
    self.feedbackView.feedbackLabelText = text;
}

@end
