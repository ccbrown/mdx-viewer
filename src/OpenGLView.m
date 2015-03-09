#if TARGET_OS == MAC_OS_X

#import "OpenGLView.h"

#include "Window.h"
#include "Controls.h"
#include "Render.h"

#import <Cocoa/Cocoa.h>

@implementation OpenGLView

- (id)initWithFrame:(NSRect)frameRect pixelFormat:(NSOpenGLPixelFormat *)format {
	if (self = [super initWithFrame:frameRect pixelFormat:format]) {
		isDragging = NO;
		[self setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
		NSTrackingArea* trackingArea = [[NSTrackingArea alloc] initWithRect:NSZeroRect options:NSTrackingMouseEnteredAndExited|NSTrackingMouseMoved|NSTrackingInVisibleRect|NSTrackingActiveInKeyWindow|NSTrackingEnabledDuringMouseDrag owner:self userInfo:nil];
		[self addTrackingArea:trackingArea];
		[trackingArea release];
	}
	
	return self;
}

- (void)prepareOpenGL {
	RenderInit();
}

+ (NSOpenGLPixelFormat*)basicPixelFormat {
	NSOpenGLPixelFormatAttribute attributes[] = {
		NSOpenGLPFANoRecovery,
		NSOpenGLPFAWindow,
		NSOpenGLPFAAccelerated,
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAColorSize, 24,
		NSOpenGLPFAAlphaSize, 8,
		NSOpenGLPFADepthSize, 32,
		NSOpenGLPFAAccumSize, 0,
		nil,
	};
	
	return [[[NSOpenGLPixelFormat alloc] initWithAttributes:attributes] autorelease];
}

- (IBAction)fullScreen:(id)sender {
	gOpenGLMutex.lock();
	if ([self isInFullScreenMode]) {
		[self exitFullScreenModeWithOptions:nil];
	} else {
		[self enterFullScreenMode:[[self window] screen] withOptions:nil];
	}
	NSSize size = [self window].frame.size;
	gViewWidth = size.width;
	gViewHeight = size.height;
	gOpenGLMutex.unlock();
}

- (BOOL)acceptsFirstResponder {
	return YES;
}

- (void)keyDown:(NSEvent*)theEvent {
	if ([theEvent isARepeat]) {
		return;
	}
	if ([theEvent keyCode] == 0) { // A
		ControlKeyPressed(ControlKeyLeft);
	} else if ([theEvent keyCode] == 1) { // S
		ControlKeyPressed(ControlKeyBackward);
	} else if ([theEvent keyCode] == 2) { // D
		ControlKeyPressed(ControlKeyRight);
	} else if ([theEvent keyCode] == 13) { // W
		ControlKeyPressed(ControlKeyForward);
	} else if ([theEvent keyCode] == 49) { // Space
		ControlKeyPressed(ControlKeyUp);
	} else if ([theEvent keyCode] == 7) { // X
		ControlKeyPressed(ControlKeyDown);
	} else if ([theEvent keyCode] == 48) { // Tab
		ToggleWireframe();
	}
}

- (void)keyUp:(NSEvent*)theEvent {
	if ([theEvent isARepeat]) {
		return;
	}
	if ([theEvent keyCode] == 0) { // A
		ControlKeyReleased(ControlKeyLeft);
	} else if ([theEvent keyCode] == 1) { // S
		ControlKeyReleased(ControlKeyBackward);
	} else if ([theEvent keyCode] == 2) { // D
		ControlKeyReleased(ControlKeyRight);
	} else if ([theEvent keyCode] == 13) { // W
		ControlKeyReleased(ControlKeyForward);
	} else if ([theEvent keyCode] == 49) { // Space
		ControlKeyReleased(ControlKeyUp);
	} else if ([theEvent keyCode] == 7) { // X
		ControlKeyReleased(ControlKeyDown);
	}
}

- (void)mouseDown:(NSEvent *)theEvent {
	isDragging = YES;

	lastWindowPoint = [theEvent locationInWindow];
}

- (void)mouseDragged:(NSEvent *)theEvent {
	if (!isDragging) {
		return;
	}
	NSPoint windowPoint = [theEvent locationInWindow];
	double dx = windowPoint.x - lastWindowPoint.x;
	double dy = windowPoint.y - lastWindowPoint.y;
	TurnRenderCamera(dx, dy);
	lastWindowPoint = windowPoint;
}

- (void)mouseUp:(NSEvent *)theEvent {
	isDragging = NO;
}

- (void)mouseExited:(NSEvent *)theEvent {
	isDragging = NO;
}

@end

#endif