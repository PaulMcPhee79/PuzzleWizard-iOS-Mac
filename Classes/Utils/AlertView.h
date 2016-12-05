
class IEventListener;

@interface AlertView : NSObject <UIAlertViewDelegate>
{
    BOOL mIsShowing;
    BOOL mDidShow;
    int mTag;
    IEventListener* mListener;
    UIAlertView* mAlertView;
}

@property (nonatomic, assign) int tag;

- (id)initWithListener:(IEventListener *)listener cancelButton:(BOOL)cancelButton;
- (void)setTitle:(NSString *)titleKey;
- (void)setMessage:(NSString *)messageKey;
- (void)addButtonWithTitle:(NSString *)titleKey;
- (void)show;

@end
