#if TARGET_OS == MAC_OS_X

#import "CocoaWindow.h"

#include "Render.h"
#include "Window.h"
#include "main.h"

#import <Cocoa/Cocoa.h>

@implementation CocoaWindow

- (id)init {
	NSRect screenRect = [[NSScreen mainScreen] visibleFrame];

	NSRect contentRect = NSMakeRect(screenRect.origin.x + screenRect.size.width * 0.5 - 400.0, screenRect.origin.x + screenRect.size.height * 0.5 - 300.0, 800.0, 600.0);
	NSUInteger windowStyle = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask;

	if ((self = [super initWithContentRect:contentRect styleMask:windowStyle backing:NSBackingStoreBuffered defer:NO])) {
		self.delegate = self;

		NSSize size = [[self contentView] frame].size;
		gViewWidth = size.width;
		gViewHeight = size.height;
	}
		
	return self;
}

- (void)windowDidResize:(NSNotification *)notification {
	NSSize size = [[self contentView] frame].size;
	gViewWidth = size.width;
	gViewHeight = size.height;
}

- (void)windowWillClose:(NSNotification *)notification {
	Terminate();
}

- (void)hideControls {
	gOpenGLMutex.lock();
	[self setStyleMask:NSBorderlessWindowMask];
	gOpenGLMutex.unlock();
}

- (void)showControls {
	gOpenGLMutex.lock();
	NSUInteger windowStyle = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask;
	[self setStyleMask:windowStyle];
	gOpenGLMutex.unlock();
}

@end

#endif