
class IEventListener;

@interface AlertViewNS : NSObject // <NSAlertDelegate>
{
    BOOL mIsShowing;
    BOOL mDidShow;
    int mTag;
    IEventListener* mListener;
    NSAlert* mAlert;
}

@property (nonatomic, assign) int tag;

- (id)initWithListener:(IEventListener *)listener cancelButton:(BOOL)cancelButton;
- (void)setAlertStyle:(NSAlertStyle)style;
- (void)setTitle:(NSString *)titleKey;
- (void)setMessage:(NSString *)messageKey;
- (void)addButtonWithTitle:(NSString *)titleKey;
- (void)show;

@end
