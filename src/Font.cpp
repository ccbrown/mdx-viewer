#include "Font.h"
#include "Utility.h"
#include "Render.h"
#include "Window.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <stdio.h>
#include <string.h>
#include <boost/thread.hpp>

bool FontKeyComp(char* lhs, char* rhs) {
	return strcmp(lhs, rhs) < 0;
}

std::map<char*, Font*, bool(*)(char*, char*)> gFonts(FontKeyComp);

float ChooseRealFontSize(float size) {
	return ceil(size / 20.0) * 20.0;
}

char* AppendFloatBytes(char* key, float f) {
	char* newKey = (char*)malloc(strlen(key) + sizeof(f) * 2 + 2);
	strcpy(newKey, key);
	int l = strlen(newKey);
	newKey[l++] = '\\';
	for (int i = 0; i < sizeof(f); ++i) {
		newKey[l++] = 'a' + (((unsigned char*)&f)[i] & 0x0F);
		newKey[l++] = 'a' + ((((unsigned char*)&f)[i] & 0xF0) >> 4);
	}
	newKey[l] = '\0';
	free(key);
	return newKey;
};

Font* FontFromFile(const char* file, float size) {
	char* ext = strrchr(file, '.');
	if (!strcmp(ext, ".ttf")) {
		return FontFromTTF(file, size);
	} else {
		return NULL;
	}
}

Font* FontFromTTF(const char* file, float size) {
	char* key = FileKey(file);
	
	if (key == NULL) {
		return NULL;
	}

	size = ChooseRealFontSize(size);
	key = AppendFloatBytes(key, size);
	
	Font* font = gFonts[key];

	if (font != NULL) {
		free(key);
		return font;
	}
	
	FT_Library library;
	if (FT_Init_FreeType(&library)) {
		printf("FT_Init_FreeType failed\n");
		free(key);
		return NULL;
	}

	char* path = (char*)malloc(strlen(gExecutableDirectory) + strlen(file) + 2);
	sprintf(path, "%s/%s", gExecutableDirectory, file);

	FT_Face face;
	if (FT_New_Face(library, path, 0, &face)) {
		printf("FT_New_Face failed loading %s\n", path);
		free(key);
		free(path);
		return NULL;
	}
	free(path);

	FT_Set_Char_Size(face, size * 64, size * 64, 96, 96);

	GLuint* textures = (GLuint*)malloc(sizeof(GLuint) * 256);

	gOpenGLMutex.lock();

	GLuint listBase = glGenLists(256);
	glGenTextures(256, textures);
	
	float* widths = (float*)malloc(sizeof(float) * 256);

	for (int i = 0; i < 256; ++i) {
		if (!MakeDisplayListForCharacter(face, (unsigned char)i, listBase, textures)) {
			glDeleteLists(listBase, 256);
			glDeleteTextures(256, textures);
			free(key);
			free(textures);
			return false;
		}
		widths[i] = face->glyph->advance.x / 64.0;
	}

	gOpenGLMutex.unlock();

	FT_Done_Face(face);

	FT_Done_FreeType(library);

	gFonts[key] = font = (Font*)malloc(sizeof(Font));
	font->listBase = listBase;
	font->textures = textures;
	font->size = size;
	font->ascender = face->ascender / 64.0;
	font->descender = face->descender / 64.0;
	font->height = face->height / 64.0;
	font->widths = widths;
	font->key = key;
	font->retainCount = 0;
	
	return font;
}

bool MakeDisplayListForCharacter(FT_Face face, unsigned char ch, GLuint listBase, GLuint* textures) {
	if(FT_Load_Glyph(face, FT_Get_Char_Index(face, ch), FT_LOAD_DEFAULT)) {
		printf("FT_Load_Glyph failed for character %d\n", ch);
		return false;
	}

	FT_Glyph glyph;
	if (FT_Get_Glyph(face->glyph, &glyph)) {
		printf("FT_Get_Glyph failed for character %d\n", ch);
		return false;
	}

	FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, 0, 1);
	FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

	FT_Bitmap& bitmap = bitmap_glyph->bitmap;

	int width = NextPowerOf2(bitmap.width);
	int height = NextPowerOf2(bitmap.rows);

	GLubyte* expanded_data = (GLubyte*)malloc(sizeof(GLubyte) * 2 * width * height);

	for (int j = 0; j < height; ++j) {
		for (int i = 0; i < width; ++i) {
			expanded_data[2 * (i + j * width)] = expanded_data[2 * (i + j * width) + 1] = (i >= bitmap.width || j >= bitmap.rows) ? 0 : bitmap.buffer[i + bitmap.width * j];
		}
	}
	
	glBindTexture(GL_TEXTURE_2D, textures[ch]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, expanded_data);

	free(expanded_data);

	glNewList(listBase + ch, GL_COMPILE);

	glBindTexture(GL_TEXTURE_2D, textures[ch]);

	glPushMatrix();

	glTranslatef(bitmap_glyph->left, bitmap_glyph->top-bitmap.rows, 0.0);

	float x = (float)bitmap.width / (float)width;
	float y = (float)bitmap.rows / (float)height;

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(0.0, bitmap.rows, 0.0);
	glTexCoord2f(0.0, y);
	glVertex3f(0.0, 0.0, 0.0);
	glTexCoord2f(x, y);
	glVertex3f(bitmap.width, 0.0, 0.0);
	glTexCoord2f(x, 0.0);
	glVertex3f(bitmap.width, bitmap.rows, 0.0);
	glEnd();
	
	glPopMatrix();
	glTranslatef(face->glyph->advance.x >> 6, 0.0, 0.0);

	glEndList();

	return true;
}

void ReleaseFont(Font* font) {
	if (font != NULL && --font->retainCount <= 0) {
		gFonts.erase(font->key);
		gOpenGLMutex.lock();
		glDeleteLists(font->listBase, 256);
		glDeleteTextures(256, font->textures);
		gOpenGLMutex.unlock();
		free(font->textures);
		free(font->widths);
		free(font->key);
		free(font);
	}
}

void RetainFont(Font* font) {
	if (font != NULL) {
		++font->retainCount;
	}
}

void FontPrint(Font* font, float size, const char* text, int n, float x, float y, float z) {	
	float modelview_matrix[16];     
	glGetFloatv(GL_MODELVIEW_MATRIX, modelview_matrix);

	float scale = (size / font->size);

	glPushMatrix();
	glLoadIdentity();
	glTranslatef(x, y, z);	
	glScalef(scale / gViewWidth, scale / gViewHeight, scale);
	glMultMatrixf(modelview_matrix);

	glListBase(font->listBase);
	glCallLists(n, GL_UNSIGNED_BYTE, text);

	glPopMatrix();
}

float FontStringWidth(Font* font, float size, const char* string, int n) {
	float sum = 0.0;
	for (int i = 0; i < n; ++i) {
		sum += font->widths[string[i]];
	}
	return sum * size / font->size;
}
