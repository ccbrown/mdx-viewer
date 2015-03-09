#ifndef FONT_H
#define FONT_H
#include "main.h";

#include <ft2build.h>
#include FT_FREETYPE_H

#include <OpenGL/gl.h>
#include <map>

struct Font {
	GLuint* textures;
	GLuint listBase;
	float size;
	float* widths;
	
	float ascender;
	float descender;
	float height;

	char* key;
	int retainCount;
};

extern std::map<char*, Font*, bool(*)(char*, char*)> gFonts;

Font* FontFromFile(const char* file, float size);
Font* FontFromTTF(const char* file, float size);

bool MakeDisplayListForCharacter(FT_Face face, unsigned char ch, GLuint listBase, GLuint* textures);

void ReleaseFont(Font* font);
void RetainFont(Font* font);

void FontPrint(Font* font, float size, const char* text, int n, float x, float y, float z);

float FontStringWidth(Font* font, float size, const char* string, int n);

#endif