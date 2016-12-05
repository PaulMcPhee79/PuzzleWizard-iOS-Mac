
#import "TFWelcomeView.h"
#import <UIKit/UIKit.h>

@interface TFWelcomeView ()

- (void)setupView;

@end


@implementation TFWelcomeView

- (id)initWithFrame:(CGRect)aRect
{
	if (self = [super initWithFrame:aRect])
    {
        [self setupView];
	}
	return self;
}

- (void)setupView {
    float marginX = 20, contentHeight = TF_ruiScale(254);
    
    mWelcomeLabel = [[[UILabel alloc] initWithFrame:CGRectMake(marginX, (self.frame.size.height - contentHeight) / 2, self.frame.size.width - 2 * marginX, TF_ruiScale(72))] autorelease];
    mWelcomeLabel.font = [UIFont boldSystemFontOfSize:TF_ruiScale(28)];
    mWelcomeLabel.textAlignment = NSTextAlignmentCenter;
    mWelcomeLabel.lineBreakMode = NSLineBreakByWordWrapping;
    mWelcomeLabel.numberOfLines = 0;
    mWelcomeLabel.textColor = [UIColor colorWithRed:90.0/255.0 green:64.0/255.0 blue:210.0/255.0 alpha:1];
    mWelcomeLabel.text = @"Welcome to Puzzle Wizard's closed beta!";
    [self addSubview:mWelcomeLabel];
    
    mInstructionsLabel = [[[UILabel alloc] initWithFrame:CGRectMake(marginX,
                                                                    mWelcomeLabel.frame.origin.y + mWelcomeLabel.frame.size.height,
                                                                    self.frame.size.width - 2 * marginX,
                                                                    TF_ruiScale(96))]
                          autorelease];
    mInstructionsLabel.font = [UIFont systemFontOfSize:TF_ruiScale(20)];
    mInstructionsLabel.textAlignment = NSTextAlignmentCenter;
    mInstructionsLabel.lineBreakMode = NSLineBreakByWordWrapping;
    mInstructionsLabel.numberOfLines = 0;
    mInstructionsLabel.text = @"Please enter your name or a pseudonym so that we may process feedback.";
    [self addSubview:mInstructionsLabel];
    
    mNameLabel = [[[UILabel alloc] initWithFrame:CGRectMake(marginX,
                                                            mInstructionsLabel.frame.origin.y + mInstructionsLabel.frame.size.height,
                                                            0.25f * self.frame.size.width - marginX,
                                                            TF_ruiScale(26))]
                 autorelease];
    mNameLabel.font = [UIFont boldSystemFontOfSize:TF_ruiScale(20)];
    mNameLabel.textAlignment = NSTextAlignmentRight;
    mNameLabel.text = @"Name:";
    [self addSubview:mNameLabel];
    
    mUsernameTextfield = [[[UITextField alloc] initWithFrame:CGRectMake(mNameLabel.frame.origin.x + mNameLabel.frame.size.width + 10,
                                                                        mNameLabel.frame.origin.y - TF_ruiScale(5),
                                                                        0.5f * self.frame.size.width,
                                                                        TF_ruiScale(36))]
                          autorelease];
    mUsernameTextfield.autocapitalizationType = UITextAutocapitalizationTypeNone;
    mUsernameTextfield.autocorrectionType = UITextAutocorrectionTypeNo;
    mUsernameTextfield.returnKeyType = UIReturnKeyDone;
    mUsernameTextfield.font = [UIFont systemFontOfSize:TF_ruiScale(24)];
    //CGSize usernameSize = [mUsernameTextfield.text sizeWithFont:[UIFont systemFontOfSize:24]];
    //CGRect usernameFrame = mUsernameTextfield.frame;
    //usernameFrame.size = CGSizeMake(usernameFrame.size.width, usernameSize.height);
    //mUsernameTextfield.frame = usernameFrame;
    mUsernameTextfield.borderStyle = UITextBorderStyleLine;
    [self addSubview:mUsernameTextfield];
    
    mSubmitButton = [UIButton buttonWithType:UIButtonTypeSystem];
    mSubmitButton.titleLabel.font = [UIFont systemFontOfSize:TF_ruiScale(20)];
    [mSubmitButton setTitle:@"Submit" forState:UIControlStateNormal];
    [mSubmitButton sizeToFit];
    mSubmitButton.center = CGPointMake(
                                       self.frame.size.width / 2,
                                       mUsernameTextfield.frame.origin.y + mUsernameTextfield.frame.size.height + mSubmitButton.frame.size.height / 2 + 10);
    [mSubmitButton addTarget:self
                      action:@selector(onSubmitPressed:)
          forControlEvents:UIControlEventTouchUpInside];
    [self addSubview:mSubmitButton];
    
    
    mWelcomeImage = [[[UIImageView alloc] initWithImage:[UIImage imageNamed:EXPUIIMG(@"cm-logo")]] autorelease];
    mWelcomeImage.frame = CGRectMake(self.frame.size.width - (mWelcomeImage.frame.size.width + 10),
                                     self.frame.size.height - (mWelcomeImage.frame.size.height + 10),
                                     mWelcomeImage.frame.size.width,
                                     mWelcomeImage.frame.size.height);
    
    [self addSubview:mWelcomeImage];
    
    //NSLog(@"WelcomeView content height: %f", (mSubmitButton.frame.origin.y + mSubmitButton.frame.size.height) - mWelcomeLabel.frame.origin.y);
}

- (NSString *)username
{
	return mUsernameTextfield.text;
}

- (void)setTextFieldDelegate:(id<UITextFieldDelegate>)tfDelegate
{
    [super setTextFieldDelegate:tfDelegate];
    mUsernameTextfield.delegate = tfDelegate;
}

@end
