#if TARGET_OS == MAC_OS_X && !defined(OPENGLVIEW_H)
#define OPENGLVIEW_H

#import <Cocoa/Cocoa.h>

@interface OpenGLView : NSOpenGLView {
	BOOL isDragging;
	NSPoint lastWindowPoint;
}

+ (NSOpenGLPixelFormat*)basicPixelFormat;

- (IBAction)fullScreen:(id)sender;

@end

#endif