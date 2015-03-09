#include "Window.h"

float gViewWidth = 0.0;
float gViewHeight = 0.0;

#if TARGET_OS == MAC_OS_X

#import <Cocoa/Cocoa.h>

#import "OpenGLView.h"
#import "CocoaWindow.h"

NSOpenGLContext* gOpenGLContext = nil;
NSWindow* gWindow = nil;

void OpenWindow() {
	CloseWindow();
		
	// NSWindows are released when closed. (see setReleasedWhenClosed:)
	gWindow  = [CocoaWindow new];
	
	NSSize size = [gWindow contentMaxSize];
	OpenGLView* glView = [[OpenGLView alloc] initWithFrame:NSMakeRect(0.0, 0.0, size.width, size.height) pixelFormat:[OpenGLView basicPixelFormat]];
	gOpenGLContext = [[glView openGLContext] retain];
	[[gWindow contentView] addSubview:glView];
	[glView release];
	
	[gWindow makeKeyAndOrderFront:nil];
}

void CloseWindow() {
	[gWindow close];
	gWindow = nil;

	[gOpenGLContext release];
	gOpenGLContext = nil;
}

#else

void OpenWindow() {
}

void CloseWindow() {
}

#endif
