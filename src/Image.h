#ifndef IMAGE_H
#define IMAGE_H

#include <OpenGL/gl.h>

bool LoadPNGImage(char* path, int &outWidth, int &outHeight, bool &outHasAlpha, GLubyte** outData);

#endif