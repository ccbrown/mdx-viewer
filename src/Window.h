#ifndef WINDOW_H
#define WINDOW_H

extern float gViewWidth;
extern float gViewHeight;

#if TARGET_OS == MAC_OS_X

#import <Cocoa/Cocoa.h>

extern NSOpenGLContext* gOpenGLContext;

#endif

void OpenWindow();
void CloseWindow();

#endif