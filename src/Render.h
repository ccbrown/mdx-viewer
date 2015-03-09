#ifndef RENDER_H
#define RENDER_H

#include <OpenGL/gl.h>
#include <boost/thread.hpp>

extern boost::mutex gOpenGLMutex;

typedef enum {
	RenderProgramNone,
	RenderProgramBasic,
	RenderProgramTextured,
	RenderProgramMDX,
	RenderProgramWMO,
	RenderProgramFinal,
	RenderProgramCount,
} RenderProgram;

void RenderInit();
void RenderDestroy();

void RenderScene();

GLuint CreateProgram(const char* name);
GLuint CreateShader(const char* filename, GLenum type);

void SetUpFramebuffer();
void SetUpRenderTextures();

void RenderFrameDidResize();

void ToggleWireframe();
void TurnRenderCamera(double dx, double dy);

void StartRenderThread();
void StopRenderThreadAndWait();

void UseRenderProgram(RenderProgram program);

void SetRenderProgramUniformi(const char* name, int value);
void SetRenderProgramUniformf(const char* name, float value);
void SetRenderProgramUniformf(const char* name, float v0, float v1, float v2, float v3);

double RenderFPS();

#endif