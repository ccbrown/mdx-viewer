#include "Camera.h"
#include "Render.h"
#include "Window.h"
#include "Controls.h"
#include "Font.h"
#include "Utility.h"
#include "World.h"
#include "matrix.h"

#include <OpenGL/gl.h>
#include <sys/time.h>
#include <list>
#include <boost/thread.hpp>

boost::mutex gOpenGLMutex;

volatile bool gStopRenderThread = false;
volatile bool gRenderThreadIsRunning = false;

long long int smoothedRenderTime = 0;

Font* gInterfaceFont = NULL;

bool gRenderWireframeScene = false;

#define NUM_FBO_COLOR_ATTACHMENTS 3
const GLenum gDFBDrawBuffers[] = {
	GL_COLOR_ATTACHMENT0_EXT,
	GL_COLOR_ATTACHMENT1_EXT,
	GL_COLOR_ATTACHMENT2_EXT,
};

GLuint gDeferredFB = 0;
GLuint gDFBDepthBuffer = 0;
GLuint gDFBTextures[NUM_FBO_COLOR_ATTACHMENTS] = {0, 0, 0};
GLuint gDFBRenderBuffers[NUM_FBO_COLOR_ATTACHMENTS] = {0, 0, 0};

GLuint gRenderPrograms[RenderProgramCount];

GLuint gRenderProgram = 0;

const char* gRenderProgramNames[] = {
	NULL,
	"gbuffers_basic",
	"gbuffers_textured",
	"gbuffers_mdx",
	"gbuffers_wmo",
	"final",
};

void RenderInit() {
	gOpenGLMutex.lock();
	
	#if TARGET_OS == MAC_OS_X
		[gOpenGLContext makeCurrentContext];
	#endif
	
	gRenderPrograms[RenderProgramNone] = 0;
	for (int i = 1; i < RenderProgramCount; ++i) {
		gRenderPrograms[i] = CreateProgram(gRenderProgramNames[i]);
	}
	RenderFrameDidResize();

	glClearDepth(1.0);
	glDepthFunc(GL_LEQUAL);
		
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	
	gOpenGLMutex.unlock();
}

void RenderScene() {	
	gOpenGLMutex.lock();

	#if TARGET_OS == MAC_OS_X
		[gOpenGLContext makeCurrentContext];
	#endif

	glViewport(0.0, 0.0, gViewWidth, gViewHeight);

	// Render scene
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, gDeferredFB);
	UseRenderProgram(RenderProgramBasic);

	glDrawBuffers(NUM_FBO_COLOR_ATTACHMENTS, gDFBDrawBuffers);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLenum depthAttachment = GL_COLOR_ATTACHMENT1_EXT;
	glDrawBuffers(1, &depthAttachment);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDrawBuffers(NUM_FBO_COLOR_ATTACHMENTS, gDFBDrawBuffers);

	Camera* cam = &(CurrentWorld()->camera);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixd(cam->projection);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixd(cam->modelView);	

	glEnable(GL_DEPTH_TEST);
	if (gRenderWireframeScene) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);		
		CurrentWorld()->render();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	} else {
		CurrentWorld()->render();
	}

	// Draw interface
	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();	

	glDisable(GL_DEPTH_TEST);

	// Draw the final scene
	
	UseRenderProgram(RenderProgramFinal);

	glBindTexture(GL_TEXTURE_2D, gDFBTextures[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gDFBTextures[1]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gDFBTextures[2]);
	glActiveTexture(GL_TEXTURE0);

	glColor4f(1.0, 1.0, 1.0, 1.0);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(0.0, 0.0, 0.0);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0, 0.0, 0.0);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0, 1.0, 0.0);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(0.0, 1.0, 0.0);
	glEnd();

	// Draw the frames
	
	UseRenderProgram(RenderProgramNone);

	glColor4f(1.0, 1.0, 1.0, 1.0);
	float fps = 1000000.0 / smoothedRenderTime;
	if (fps > 9000.0) {
		FontPrint(gInterfaceFont, 20.0, "Over 9000 FPS!", 14, 8.0 / gViewWidth, 8.0 / gViewHeight, 0.0);
	} else {
		char fpsString[20];
		int len = sprintf(fpsString, "%0.2f FPS", fps);
		FontPrint(gInterfaceFont, 20.0, fpsString, len, 8.0 / gViewWidth, 8.0 / gViewHeight, 0.0);
	}

	#if TARGET_OS == MAC_OS_X
		CGLFlushDrawable((CGLContextObj)[gOpenGLContext CGLContextObj]);
	#endif

	gOpenGLMutex.unlock();
}

GLuint CreateProgram(const char* name) {
	char* path = (char*)malloc(strlen(gExecutableDirectory) + strlen(name) + 14);

	sprintf(path, "%s/Shaders/%s.vsh", gExecutableDirectory, name);
	GLuint vsh = CreateShader(path, GL_VERTEX_SHADER);

	sprintf(path, "%s/Shaders/%s.fsh", gExecutableDirectory, name);
	GLuint fsh = CreateShader(path, GL_FRAGMENT_SHADER);
	
	GLuint program = 0;
	
	if (vsh && fsh) {
		program = glCreateProgram();
		glAttachShader(program, vsh);
		glAttachShader(program, fsh);
		glLinkProgram(program);
	}
	
	return program;
}

GLuint CreateShader(const char* filename, GLenum type) {
	GLuint shader;
	if ((shader = glCreateShader(type)) == 0) {
		printf("Couldn't create shader\n");
		return 0;
	}
	long int len = 10000;
	char* source = (char*)malloc(len);
	if (!ReadFile(filename, source, &len) || len <= 0) {
		printf("Couldn't read file %s\n", filename);
		free(source);		
		return 0;
	}
	free(source);
	
	GLint srclen = len;
	glShaderSource(shader, 1, (const GLchar**)&source, (const GLint*)&srclen);
	
	glCompileShader(shader);
	
	char* logBuffer = (char*)malloc(1000);
	GLsizei length;
	glGetShaderInfoLog(shader, 1000, &length, logBuffer);
	if (length > 0) {
		printf("%s\n", logBuffer);
	}
	free(logBuffer);
	
	return shader;
}

void SetUpFramebuffer() {
	SetUpRenderTextures();
	
	glDeleteFramebuffersEXT(1, &gDeferredFB);
	glDeleteRenderbuffersEXT(NUM_FBO_COLOR_ATTACHMENTS, gDFBRenderBuffers);
	
	glGenFramebuffersEXT(1, &gDeferredFB);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, gDeferredFB);

	glGenRenderbuffersEXT(NUM_FBO_COLOR_ATTACHMENTS, gDFBRenderBuffers);

	for (int i = 0; i < NUM_FBO_COLOR_ATTACHMENTS; ++i) {
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, gDFBRenderBuffers[i]);
		if (i == 1) { // Depth buffer
			glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGB32F_ARB, gViewWidth, gViewHeight);
		} else {
			glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA8, gViewWidth, gViewHeight);
		}
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, gDFBDrawBuffers[i], GL_RENDERBUFFER_EXT, gDFBRenderBuffers[i]);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, gDFBDrawBuffers[i], GL_TEXTURE_2D, gDFBTextures[i], 0);
	}
	
	if (gDFBDepthBuffer) {
		glDeleteFramebuffersEXT(1, &gDFBDepthBuffer);
	}

	glGenRenderbuffersEXT(1, &gDFBDepthBuffer);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, gDFBDepthBuffer);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, gViewWidth, gViewHeight);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gDFBDepthBuffer);

	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		// Todo: Error handling?
		printf("Failed creating framebuffer\n");
	}
}

void SetUpRenderTextures() {
	#if TARGET_OS == MAC_OS_X
		[gOpenGLContext makeCurrentContext];
	#endif

	glDeleteTextures(NUM_FBO_COLOR_ATTACHMENTS, gDFBTextures);
	glGenTextures(NUM_FBO_COLOR_ATTACHMENTS, gDFBTextures);
	
	for (int i = 0; i < NUM_FBO_COLOR_ATTACHMENTS; ++i) {
		glBindTexture(GL_TEXTURE_2D, gDFBTextures[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		if (i == 1) { // Depth buffer
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F_ARB, gViewWidth, gViewHeight, 0, GL_BGRA, GL_FLOAT, NULL);
		} else {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, gViewWidth, gViewHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
		}
	}
}

void ToggleWireframe() {
	gRenderWireframeScene = !gRenderWireframeScene;
}

void TurnRenderCamera(double dx, double dy) {
	Camera* cam = &(CurrentWorld()->camera);
	cam->turnRight(dx * 0.002);
	cam->turnUp(dy * 0.002);
	cam->calculateModelView();
}

void RenderFrameDidResize() {
	if (!CurrentWorld()) {
		return;
	}
	Camera* cam = &(CurrentWorld()->camera);
	cam->aspectRatio = gViewWidth / gViewHeight;
	cam->calculateProjection();
	SetUpFramebuffer();
}

void RenderThreadEntry() {
	#if TARGET_OS == MAC_OS_X
		[gOpenGLContext makeCurrentContext];
	#endif
	
	ReleaseFont(gInterfaceFont);
	gInterfaceFont = FontFromFile("/Fonts/Arial.ttf", 20.0);
	if (gInterfaceFont != NULL) {
		RetainFont(gInterfaceFont);
	}

	RenderFrameDidResize();

	int lastWidth = 0;
	int lastHeight = 0;
		
	while (!gStopRenderThread) {
		timeval startTime;
		gettimeofday(&startTime, NULL);
		clock_t clockStart = clock();
		if (gViewWidth != lastWidth || gViewHeight != lastHeight) {
			RenderFrameDidResize();
			lastWidth  = gViewWidth;
			lastHeight = gViewHeight;
		}
		RenderScene();
		timeval endTime;
		gettimeofday(&endTime, NULL);
		timeval diff;
		diff.tv_sec = endTime.tv_sec - startTime.tv_sec;
		diff.tv_usec = endTime.tv_usec - startTime.tv_usec;
		long long int renderTime = diff.tv_sec * 1000000.0 + diff.tv_usec;
		UpdateCamera(&(CurrentWorld()->camera), renderTime);
		smoothedRenderTime = smoothedRenderTime * 0.8 + renderTime * 0.2;
	}

	ReleaseFont(gInterfaceFont);
	gInterfaceFont = NULL;

	gRenderThreadIsRunning = false;
}

void StartRenderThread() {
	StopRenderThreadAndWait();	
	
	gRenderThreadIsRunning = true;
	
	boost::thread(boost::ref(RenderThreadEntry));
}

void StopRenderThreadAndWait() {
	gStopRenderThread = true;
	while (gRenderThreadIsRunning) { }
	gStopRenderThread = false;
}

void UseRenderProgram(RenderProgram program) {
	if (program >= RenderProgramCount) {
		return;
	}	
	
	GLuint prog = gRenderPrograms[program];
	
	if (prog == gRenderProgram) {
		return;
	}
	
	glUseProgram(prog);
	gRenderProgram = prog;
	
	// extra program-dependent setup
	switch (program) {
		case RenderProgramTextured: {
			GLint utexture = glGetUniformLocation(prog, "texture");
			glUniform1i(utexture, 0);
			break;
		}
		case RenderProgramMDX: {
			GLint utexture = glGetUniformLocation(prog, "texture");
			glUniform1i(utexture, 0);
			break;
		}
		case RenderProgramWMO: {
			GLint utexture1 = glGetUniformLocation(prog, "texture1");
			glUniform1i(utexture1, 0);
			GLint utexture2 = glGetUniformLocation(prog, "texture2");
			glUniform1i(utexture2, 1);
			break;
		}
		case RenderProgramFinal: {
			GLint ugcolor = glGetUniformLocation(prog, "gcolor");
			glUniform1i(ugcolor, 0);
			GLint ugdepth = glGetUniformLocation(prog, "gdepth");
			glUniform1i(ugdepth, 1);
			GLint ugnormal = glGetUniformLocation(prog, "gnormal");
			glUniform1i(ugnormal, 2);
			break;
		}
	}
}

void SetRenderProgramUniformi(const char* name, int value) {
	GLint ui = glGetUniformLocation(gRenderProgram, name);
	glUniform1i(ui, value);
}

void SetRenderProgramUniformf(const char* name, float value) {
	GLint ui = glGetUniformLocation(gRenderProgram, name);
	glUniform1f(ui, value);
}

void SetRenderProgramUniformf(const char* name, float v0, float v1, float v2, float v3) {
	GLint ui = glGetUniformLocation(gRenderProgram, name);
	glUniform4f(ui, v0, v1, v2, v3);
}

double RenderFPS() {
	return 1000000.0 / double(smoothedRenderTime);
}

void RenderDestroy() {
	StopRenderThreadAndWait();
	
	glDeleteFramebuffersEXT(1, &gDeferredFB);
	glDeleteFramebuffersEXT(1, &gDFBDepthBuffer);
	glDeleteTextures(NUM_FBO_COLOR_ATTACHMENTS, gDFBTextures);
	
	for (int i = 1; i < RenderProgramCount; ++i) {
		glDeleteProgram(gRenderPrograms[i]);
	}
}
