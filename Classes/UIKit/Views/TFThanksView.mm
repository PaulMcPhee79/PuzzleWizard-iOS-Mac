
#import "TFThanksView.h"
#import <UIKit/UIKit.h>

@interface TFThanksView ()

- (void)setupView;

@end


@implementation TFThanksView

- (id)initWithFrame:(CGRect)aRect
{
	if (self = [super initWithFrame:aRect])
    {
        [self setupView];
	}
	return self;
}

- (void)setupView {
    float marginX = 40, contentHeight = TF_ruiScale(280);
    
    mIntroLabel = [[[UILabel alloc] initWithFrame:CGRectMake(marginX, (self.frame.size.height - contentHeight) / 2, self.frame.size.width - 2 * marginX, TF_ruiScale(60))] autorelease];
    mIntroLabel.font = [UIFont systemFontOfSize:TF_ruiScale(17)];
    mIntroLabel.textAlignment = NSTextAlignmentLeft;
    mIntroLabel.lineBreakMode = NSLineBreakByWordWrapping;
    mIntroLabel.numberOfLines = 0;
    mIntroLabel.text = @"Thanks for beta testing Puzzle Wizard! Your playtesting and feedback has been a huge help.";
    [self addSubview:mIntroLabel];
    
    mAnnouncementLabel = [[[UILabel alloc] initWithFrame:CGRectMake(marginX,
                                                                    mIntroLabel.frame.origin.y + mIntroLabel.frame.size.height,
                                                                    self.frame.size.width - 2 * marginX,
                                                                    TF_ruiScale(140))]
                          autorelease];
    mAnnouncementLabel.font = [UIFont systemFontOfSize:TF_ruiScale(17)];
    mAnnouncementLabel.textAlignment = NSTextAlignmentLeft;
    mAnnouncementLabel.lineBreakMode = NSLineBreakByWordWrapping;
    mAnnouncementLabel.numberOfLines = 0;
    mAnnouncementLabel.text = @"Keep an eye out for the TouchArcade Puzzle Wizard release thread - we'd love to have experienced players participating in the discussion! As a token of our appreciation, PM user 'CheekyMammoth' on the TouchArcade forums to claim your promo code upon release.";
    [self addSubview:mAnnouncementLabel];
    
    mFinalLabel = [[[UILabel alloc] initWithFrame:CGRectMake(marginX,
                                                             mAnnouncementLabel.frame.origin.y + mAnnouncementLabel.frame.size.height + 4,
                                                             self.frame.size.width - 2 * marginX,
                                                             TF_ruiScale(32))]
                   autorelease];
    mFinalLabel.font = [UIFont boldSystemFontOfSize:TF_ruiScale(26)];
    mFinalLabel.textAlignment = NSTextAlignmentCenter;
    mFinalLabel.lineBreakMode = NSLineBreakByWordWrapping;
    mFinalLabel.numberOfLines = 0;
    mFinalLabel.textColor = [UIColor colorWithRed:90.0/255.0 green:64.0/255.0 blue:210.0/255.0 alpha:1];
    mFinalLabel.text = @"    Thank You! :-)";
    [self addSubview:mFinalLabel];
    
    mCloseButton = [UIButton buttonWithType:UIButtonTypeSystem];
    mCloseButton.titleLabel.font = [UIFont systemFontOfSize:TF_ruiScale(20)];
    [mCloseButton setTitle:@"Close" forState:UIControlStateNormal];
    [mCloseButton sizeToFit];
    mCloseButton.center = CGPointMake(
                                      self.frame.size.width / 2,
                                      mFinalLabel.frame.origin.y + mFinalLabel.frame.size.height + mCloseButton.frame.size.height / 2 + 10);
    [mCloseButton addTarget:self
                     action:@selector(onSubmitPressed:)
           forControlEvents:UIControlEventTouchUpInside];
    [self addSubview:mCloseButton];
    
    mThanksImage = [[[UIImageView alloc] initWithImage:[UIImage imageNamed:EXPUIIMG(@"cm-logo")]] autorelease];
    mThanksImage.frame = CGRectMake(self.frame.size.width - (mThanksImage.frame.size.width + 10),
                                     self.frame.size.height - (mThanksImage.frame.size.height + 10),
                                     mThanksImage.frame.size.width,
                                     mThanksImage.frame.size.height);
    
    [self addSubview:mThanksImage];
    
    //NSLog(@"ThanksView content height: %f", (mCloseButton.frame.origin.y + mCloseButton.frame.size.height) - mIntroLabel.frame.origin.y);
}

@end
