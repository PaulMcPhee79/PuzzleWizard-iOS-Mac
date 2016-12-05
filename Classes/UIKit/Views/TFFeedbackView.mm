
#import "TFFeedbackView.h"
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

@interface TFFeedbackView ()

- (void)setupView;

@end


@implementation TFFeedbackView

- (id)initWithFrame:(CGRect)aRect
{
	if (self = [super initWithFrame:aRect])
    {
		[self setupView];
	}
	return self;
}

- (void)setupView
{
	float marginX = 20;
    
    mHeaderLabel = [[[UILabel alloc] initWithFrame:CGRectMake(0, 20, self.frame.size.width, 36)] autorelease];
    mHeaderLabel.font = [UIFont systemFontOfSize:28];
    mHeaderLabel.textAlignment = NSTextAlignmentCenter;
    mHeaderLabel.lineBreakMode = NSLineBreakByWordWrapping;
    mHeaderLabel.numberOfLines = 0;
    mHeaderLabel.textColor = [UIColor colorWithRed:90.0/255.0 green:64.0/255.0 blue:210.0/255.0 alpha:1];
    mHeaderLabel.text = @"Congrats on completing Level X!";
    [self addSubview:mHeaderLabel];
    
    mIntroLabel = [[[UILabel alloc] initWithFrame:CGRectMake(marginX,
                                                             mHeaderLabel.frame.origin.y + mHeaderLabel.frame.size.height + 30,
                                                             self.frame.size.width - 2 * marginX,
                                                             48)]
                   autorelease];
    mIntroLabel.font = [UIFont systemFontOfSize:20];
    mIntroLabel.textAlignment = NSTextAlignmentLeft;
    mIntroLabel.lineBreakMode = NSLineBreakByWordWrapping;
    mIntroLabel.numberOfLines = 0;
    mIntroLabel.text = @"You can help make Puzzle Wizard even better by submitting feedback on the following:";
    [self addSubview:mIntroLabel];
    
    mQuestionLabel = [[[UILabel alloc] initWithFrame:CGRectMake(marginX,
                                                                mIntroLabel.frame.origin.y + mIntroLabel.frame.size.height + 20,
                                                                0.9f * self.frame.size.width - 2 * marginX,
                                                                48)]
                      autorelease];
    mQuestionLabel.font = [UIFont boldSystemFontOfSize:18];
    mQuestionLabel.textAlignment = NSTextAlignmentLeft;
    mQuestionLabel.lineBreakMode = NSLineBreakByWordWrapping;
    mQuestionLabel.numberOfLines = 0;
    mQuestionLabel.text = @"Q. How would you explain Conveyor Belt to a friend who hasn't played Puzzle Wizard?";
    [self addSubview:mQuestionLabel];
    
    // FIXME: iOS7+
    mQuestionTextView = [[[UITextView alloc] initWithFrame:CGRectMake(mQuestionLabel.frame.origin.x,
                                                                      mQuestionLabel.frame.origin.y + mQuestionLabel.frame.size.height,
                                                                      self.frame.size.width - 2 * marginX,
                                                                      96)
                                             textContainer:nil]
                         autorelease];
    mQuestionTextView.layer.borderWidth = 2.0f;
    mQuestionTextView.layer.borderColor = [[UIColor grayColor] CGColor];
    mQuestionTextView.font = [UIFont systemFontOfSize:20];
    [self addSubview:mQuestionTextView];
    
    mFeedbackLabel = [[[UILabel alloc] initWithFrame:CGRectMake(marginX,
                                                                mQuestionTextView.frame.origin.y + mQuestionTextView.frame.size.height + 30,
                                                                0.9f * self.frame.size.width - 2 * marginX,
                                                                48)]
                      autorelease];
    mFeedbackLabel.font = [UIFont boldSystemFontOfSize:18];
    mFeedbackLabel.textAlignment = NSTextAlignmentLeft;
    mFeedbackLabel.lineBreakMode = NSLineBreakByWordWrapping;
    mFeedbackLabel.numberOfLines = 0;
    mFeedbackLabel.text = @"Q. Do you have any specific recommendations based on this level?";
    [self addSubview:mFeedbackLabel];
    
    mFeedbackTextView = [[[UITextView alloc] initWithFrame:CGRectMake(mFeedbackLabel.frame.origin.x,
                                                                      mFeedbackLabel.frame.origin.y + mFeedbackLabel.frame.size.height,
                                                                      self.frame.size.width - 2 * marginX,
                                                                      96)
                                             textContainer:nil]
                         autorelease];
    mFeedbackTextView.layer.borderWidth = 2.0f;
    mFeedbackTextView.layer.borderColor = [[UIColor grayColor] CGColor];
    mFeedbackTextView.font = [UIFont systemFontOfSize:20];
    [self addSubview:mFeedbackTextView];
    
    mSubmitButton = [UIButton buttonWithType:UIButtonTypeSystem];
    mSubmitButton.titleLabel.font = [UIFont systemFontOfSize:24];
    [mSubmitButton setTitle:@"Submit" forState:UIControlStateNormal];
    [mSubmitButton sizeToFit];
    mSubmitButton.center = CGPointMake(
                                       self.frame.size.width / 2,
                                       mFeedbackTextView.frame.origin.y + mFeedbackTextView.frame.size.height + mSubmitButton.frame.size.height / 2 + 42);
    [mSubmitButton addTarget:self
                      action:@selector(onSubmitPressed:)
            forControlEvents:UIControlEventTouchUpInside];
    [self addSubview:mSubmitButton];
    
    mLogoImage = [[[UIImageView alloc] initWithImage:[UIImage imageNamed:EXPUIIMG(@"cm-logo")]] autorelease];
    mLogoImage.frame = CGRectMake(self.frame.size.width - (mLogoImage.frame.size.width + marginX),
                                  mSubmitButton.center.y - mLogoImage.frame.size.height / 2,
                                  mLogoImage.frame.size.width,
                                  mLogoImage.frame.size.height);
    
    [self addSubview:mLogoImage];
}

- (void)setTextViewDelegate:(id<UITextViewDelegate>)tvDelegate
{
    [super setTextViewDelegate:tvDelegate];
    mQuestionTextView.delegate = mFeedbackTextView.delegate = tvDelegate;
}

- (NSString *)headerLabelText
{
	return mHeaderLabel.text;
}

- (void)setHeaderLabelText:(NSString *)text
{
    mHeaderLabel.text = text;
}

- (NSString *)questionLabelText
{
	return mQuestionLabel.text;
}

- (void)setQuestionLabelText:(NSString *)text
{
    mQuestionLabel.text = text;
}

- (NSString *)questionUserText
{
	return mQuestionTextView.text;
}

- (NSString *)feedbackLabelText
{
	return mFeedbackLabel.text;
}

- (void)setFeedbackLabelText:(NSString *)text
{
    mFeedbackLabel.text = text;
}

- (NSString *)feedbackUserText
{
	return mFeedbackTextView.text;
}

@end
