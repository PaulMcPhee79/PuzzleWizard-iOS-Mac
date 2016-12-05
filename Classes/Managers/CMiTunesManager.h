//
//  CMiTunesManager.h
//  PuzzleWizard
//
//  Created by Paul McPhee on 27/02/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface CMiTunesManager : NSObject

+ (BOOL)shouldPromptForRating;
+ (void)userRespondedToRatingPrompt:(BOOL)response;
+ (void)userDidRateTheGame:(BOOL)success;

@end
