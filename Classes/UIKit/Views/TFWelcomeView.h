
#import <Foundation/Foundation.h>
#import "TFView.h"

@interface TFWelcomeView : TFView
{
    UILabel* mWelcomeLabel;
    UILabel* mInstructionsLabel;
    UILabel* mNameLabel;
    UIImageView* mWelcomeImage;
	UITextField* mUsernameTextfield;
    UIButton* mSubmitButton;
}

@property (nonatomic,readonly) NSString *username;

@end
