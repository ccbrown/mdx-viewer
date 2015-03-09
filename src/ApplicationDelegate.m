#if TARGET_OS == MAC_OS_X

#import "ApplicationDelegate.h"
#include "main.h"

#import <Cocoa/Cocoa.h>

@implementation ApplicationDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	Initialize([[[[NSBundle mainBundle] bundleURL] relativePath] UTF8String]);
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication {
	return YES;
}

@end

#endif