#if TARGET_OS == MAC_OS_X && !defined(COCOAWINDOW_H)
#define COCOAWINDOW_H

#import <Cocoa/Cocoa.h>

@interface CocoaWindow : NSWindow <NSWindowDelegate> {
}

- (void)windowDidResize:(NSNotification *)notification;
- (void)windowWillClose:(NSNotification *)notification;

- (void)hideControls;
- (void)showControls;

@end

#endif