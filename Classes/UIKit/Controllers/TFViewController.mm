
#import "TFViewController.h"
#import "cocos2d.h"
USING_NS_CC;

@implementation TFViewController

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    [self.view endEditing:YES];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
	return UIInterfaceOrientationIsLandscape(toInterfaceOrientation);
    //return toInterfaceOrientation == UIInterfaceOrientationPortrait;
}

- (void)viewDidDisappear:(BOOL)animated
{
    [super viewDidDisappear:animated];
    
    CCDirector::sharedDirector()->resume();
    CCDirector::sharedDirector()->startAnimation();
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    //self.automaticallyAdjustsScrollViewInsets = NO;
    CGRect screenRect = [UIScreen mainScreen].applicationFrame;
    scrollView = [[[TFScrollView alloc] initWithFrame:CGRectMake(0, 0, screenRect.size.height, screenRect.size.width)] autorelease];
    
    //scroll view must be under main view - swap it
    UIView* natView = self.view;
    [self setView:scrollView];
    [self.view addSubview:natView];
    
    CGSize scrollViewContentSize = natView.frame.size;
    [scrollView setContentSize:scrollViewContentSize];
    
    [self registerForKeyboardNotifications];
}

- (void)viewDidUnload
{
    [activeField release]; activeField = nil;
    [self unregisterForKeyboardNotifications];
    [super viewDidUnload];
}

- (void)viewDidLayoutSubviews
{
    [super viewDidLayoutSubviews];
    CGSize scrollViewContentSize = ((UIView *)self.view.subviews[0]).frame.size;
    [scrollView setContentSize:scrollViewContentSize];
}

- (void)registerForKeyboardNotifications
{
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(keyboardWillShow:)
                                                 name:UIKeyboardWillShowNotification object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(keyboardWillBeHidden:)
                                                 name:UIKeyboardWillHideNotification object:nil];
    
}

-(void)unregisterForKeyboardNotifications
{
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:UIKeyboardWillShowNotification
                                                  object:nil];
    // unregister for keyboard notifications while not visible.
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:UIKeyboardWillHideNotification
                                                  object:nil];
}

- (void)keyboardWillShow:(NSNotification*)aNotification
{
    if (keyboardIsShown || !self.isViewLoaded || !self.view.window)
        return;
    
    NSDictionary* info = [aNotification userInfo];
    CGRect kbFrameInWindow = [[info objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue];
    CGRect kbFrameInView = [self.view convertRect:kbFrameInWindow fromView:nil];
    
    CGRect activeFieldFrame = activeField.frame;
    CGRect afFrameInView = [self.view convertRect:activeFieldFrame fromView:(UIView *)self.view.subviews[0]];
    float yOffset = (afFrameInView.origin.y + afFrameInView.size.height) - kbFrameInView.origin.y;
    UIEdgeInsets newContentInsets = UIEdgeInsetsMake(0, 0, MAX(0, yOffset), 0); // top, left, bottom, right
    
    CGRect newBounds = scrollView.bounds;
    newBounds.origin.y += yOffset;
    NSTimeInterval animationDuration;
    UIViewAnimationCurve animationCurve;
    [[info objectForKey:UIKeyboardAnimationDurationUserInfoKey] getValue:&animationDuration];
    [[info objectForKey:UIKeyboardAnimationCurveUserInfoKey] getValue:&animationCurve];
    [UIView animateWithDuration:animationDuration
                          delay:0
                        options:(UIViewAnimationOptions)((NSUInteger)animationCurve << 16)
                     animations:^{
                         scrollView.contentInset = newContentInsets;
                         scrollView.scrollIndicatorInsets = newContentInsets;
                         scrollView.bounds = newBounds;
                     } completion:nil];

    
//    CGRect frame = scrollView.frame;
//    frame.size.height -= kbSize.height;
//    
//    CGPoint fOrigin = activeField.frame.origin;
//    fOrigin.y -= scrollView.contentOffset.y;
//    fOrigin.y += activeField.frame.size.height;
//    if (!CGRectContainsPoint(frame, fOrigin) )
//    {
//        CGPoint scrollPoint = CGPointMake(0.0, activeField.frame.origin.y + activeField.frame.size.height - frame.size.height); // 60
//        NSTimeInterval animationDuration;
//        UIViewAnimationCurve animationCurve;
//        [[info objectForKey:UIKeyboardAnimationDurationUserInfoKey] getValue:&animationDuration];
//        [[info objectForKey:UIKeyboardAnimationCurveUserInfoKey] getValue:&animationCurve];
//        [UIView animateWithDuration:animationDuration
//                              delay:0
//                            options:(UIViewAnimationOptions)((NSUInteger)animationCurve << 16)
//                         animations:^{
//                             [scrollView setContentOffset:scrollPoint animated:NO];
//                         } completion:nil];
//    }
    
    keyboardIsShown = YES;
}

- (void)keyboardWillBeHidden:(NSNotification*)aNotification
{
    if (!keyboardIsShown || !self.isViewLoaded || !self.view.window)
        return;
    
    NSDictionary* info = [aNotification userInfo];
    
    NSTimeInterval animationDuration;
    UIViewAnimationCurve animationCurve;
    [[info objectForKey:UIKeyboardAnimationDurationUserInfoKey] getValue:&animationDuration];
    [[info objectForKey:UIKeyboardAnimationCurveUserInfoKey] getValue:&animationCurve];
    [UIView animateWithDuration:animationDuration
                          delay:0
                        options:(UIViewAnimationOptions)((NSUInteger)animationCurve << 16)
                     animations:^{
                         scrollView.contentInset = UIEdgeInsetsZero;
                         scrollView.scrollIndicatorInsets = UIEdgeInsetsZero;
                     } completion:nil];
    
    
//    CGSize kbSize = [[info objectForKey:UIKeyboardFrameBeginUserInfoKey] CGRectValue].size;
//    
//    //UIView* subview = scrollView.subviews[0];
//    CGRect viewFrame = scrollView.frame; //[scrollView convertRect:subview.frame fromView:subview];
//    viewFrame.size.width -= kbSize.width;
//    
//    NSTimeInterval animationDuration;
//    UIViewAnimationCurve animationCurve;
//    [[info objectForKey:UIKeyboardAnimationDurationUserInfoKey] getValue:&animationDuration];
//    [[info objectForKey:UIKeyboardAnimationCurveUserInfoKey] getValue:&animationCurve];
//    [UIView animateWithDuration:animationDuration
//                          delay:0
//                        options:(UIViewAnimationOptions)((NSUInteger)animationCurve << 16)
//                     animations:^{
//                         [scrollView setFrame:viewFrame];
//                     } completion:nil];
    

    //[scrollView setContentOffset:CGPointZero animated:YES];
    keyboardIsShown = NO;
}

- (BOOL)textFieldShouldBeginEditing:(UITextField *)textField
{
    activeField = [textField retain];
    return YES;
}

- (void)textFieldDidEndEditing:(UITextField *)textField
{
    [activeField release]; activeField = nil;
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
    [textField resignFirstResponder];
    return YES;
}

- (BOOL)textViewShouldBeginEditing:(UITextView *)textView
{
    activeField = [textView retain];
    return YES;
}

- (void)textViewDidEndEditing:(UITextView *)textView
{
    [activeField release]; activeField = nil;
}

@end
