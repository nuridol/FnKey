//
//  AppDelegate.h
//  K810FnKey
//
//  Created by NuRi on 1/26/14.
//  Copyright (c) 2014 NuRi. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface AppDelegate : NSObject <NSApplicationDelegate>

@property (assign) IBOutlet NSWindow *window;
@property (assign) IBOutlet NSButton *button;

- (IBAction)pushSet:(id)aSender;
@end
