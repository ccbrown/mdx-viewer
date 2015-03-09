#include "ApplicationDelegate.h"
#include "Controls.h"
#include "Render.h"
#include "Window.h"
#include "World.h"
#include "DBC.h"
#include "MPQ.h"

#include "main.h";

#if TARGET_OS == MAC_OS_X
#import <Cocoa/Cocoa.h>
#endif

char* gExecutableDirectory = NULL;

int main(int argc, char* argv[]) {	

	#if TARGET_OS == MAC_OS_X
		return NSApplicationMain(argc, (const char**)argv);
	#else
		printf("I don't know what to do for this operating system yet!\n");
	#endif
	
	return 0;
}

void Initialize(const char* directory) {
	char* lastSlash = strrchr(directory, '/');
	size_t l = lastSlash - directory;
	gExecutableDirectory = (char*)malloc(l + 1);
	memcpy(gExecutableDirectory, directory, l);
	gExecutableDirectory[l] = '\0';

	MPQInit();
	DBCInit();

	ControlsInit();

	OpenWindow();
	
	WorldInit();
	
	StartRenderThread();
}

void Terminate() {
	RenderDestroy();

	WorldDestroy();
	
	#if TARGET_OS == MAC_OS_X
		if (gOpenGLContext != nil) {
			[gOpenGLContext release];
			gOpenGLContext = nil;
		}
	#endif

	DBCDestroy();
	MPQDestroy();

	free(gExecutableDirectory);
	gExecutableDirectory = NULL;
}
