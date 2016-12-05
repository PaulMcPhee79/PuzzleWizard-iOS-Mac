
#import <Foundation/Foundation.h>
#import "TFView.h"

@interface TFFeedbackView : TFView
{
    UILabel* mHeaderLabel;
    UILabel* mIntroLabel;
    
    UILabel* mQuestionLabel;
	UITextView* mQuestionTextView;
    
    UILabel* mFeedbackLabel;
	UITextView* mFeedbackTextView;
    
    UIImageView* mLogoImage;
    UIButton* mSubmitButton;
}

@property (nonatomic,copy) NSString *headerLabelText;

@property (nonatomic,copy) NSString *questionLabelText;
@property (nonatomic,readonly) NSString *questionUserText;

@property (nonatomic,copy) NSString *feedbackLabelText;
@property (nonatomic,readonly) NSString *feedbackUserText;

@end
