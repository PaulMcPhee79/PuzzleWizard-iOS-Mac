
#import "TFScrollView.h"

@implementation TFScrollView

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	[self.nextResponder touchesBegan:touches withEvent:event];
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	[self.nextResponder touchesEnded:touches withEvent:event];
}

@end
