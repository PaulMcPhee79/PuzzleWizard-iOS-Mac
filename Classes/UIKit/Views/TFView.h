
#import <Foundation/Foundation.h>
#import "TFCommon.h"

@interface TFView : UIView
{
    TFEventHandler mOnSubmit;
}

@property (nonatomic,copy) TFEventHandler onSubmit;

- (void)setTextFieldDelegate:(id<UITextFieldDelegate>)tfDelegate;
- (void)setTextViewDelegate:(id<UITextViewDelegate>)tvDelegate;

@end
