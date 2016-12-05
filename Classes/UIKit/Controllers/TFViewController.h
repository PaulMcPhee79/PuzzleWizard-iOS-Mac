
#import <UIKit/UIKit.h>
#import "TFScrollView.h"

@interface TFViewController : UIViewController <UITextFieldDelegate, UITextViewDelegate>
{
    BOOL keyboardIsShown;
    UIView* activeField;
    TFScrollView* scrollView;
}

@end
