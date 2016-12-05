
#include "UtilsNS.h"
#include <stdio.h>
#import <Foundation/Foundation.h>

void UtilsNS::CMLog(const char* msg)
{
#if DEBUG
    if (msg)
    {
        // Commented NSLog solution fails since Yosemite.
//        NSString* str = [NSString stringWithCString:msg encoding:NSUTF8StringEncoding];
//        dispatch_sync(dispatch_get_main_queue(), ^(void) {
//            NSLog(@"%@", str);
//        });
        
        fprintf(stderr, "%s\n", msg);
        fflush(stderr);
    }
#endif
}
