
#import <Foundation/Foundation.h>
#import "DeviceDetails.h"
#import "TFCommon.h"
#include <map>
#include <string>

float TF_ruiScale(float val)
{
    return isIPad() ? val * 2 : val;
}

NSString* TF_expandUIImagePath(NSString* imageName)
{
	if (imageName == nil)
		return nil;
    
    const char* utf8ImageName = [imageName UTF8String];
    NSString* expandedPath;
    
    if ([UIScreen mainScreen])
    {
        // iOS
        std::map<int, std::string> fileExtMap;
        fileExtMap[480] = std::string("iphone/uiview/") + utf8ImageName + ".png";
        fileExtMap[960] = std::string("ipad+iphonehd/uiview/") + utf8ImageName + "@2x.png";
        fileExtMap[1136] = std::string("ipad+iphonehd/uiview/") + utf8ImageName + "@2x.png";
        fileExtMap[1024] = std::string("ipad+iphonehd/uiview/") + utf8ImageName + "@2x.png";
        fileExtMap[2048] = std::string("ipadhd/uiview/") + utf8ImageName + "@hd.png";
        
        int key = (int)([UIScreen mainScreen].scale * ([[UIScreen mainScreen] bounds].size.height + 0.01f));
        std::map<int, std::string>::iterator it = fileExtMap.find(key);
        if (it != fileExtMap.end())
        {
            expandedPath = [NSString stringWithUTF8String:it->second.c_str()];
            return expandedPath;
        }
    }
    
    expandedPath = [NSString stringWithUTF8String:(std::string("machd/nsview/") + utf8ImageName + "@hd.png").c_str()]; // Mac
    return expandedPath;
}
