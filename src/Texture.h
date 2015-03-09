#ifndef TEXTURE_H
#define TEXTURE_H

#include <map>
#include <OpenGL/gl.h>

#define BLP_TYPE_JPEG             0
#define BLP_TYPE_DIRECTX          1

#define BLP_ENCODING_UNCOMPRESSED 1
#define BLP_ENCODING_DIRECTX      2

#define BLP_ALPHA_ENCODING_DXT1   0
#define BLP_ALPHA_ENCODING_DXT2_3 1
#define BLP_ALPHA_ENCODING_DXT4_5 7

#pragma pack(push, 1)

typedef struct {
	char r;
	char g;
	char b;
	char a;
} BLPRGBAColor;

typedef struct {
	uint32_t magic;
	uint32_t type;
    uint8_t encoding;
    uint8_t alphaDepth;
    uint8_t alphaEncoding;
    uint8_t hasMips;
	uint32_t width;
	uint32_t height;
	uint32_t offsets[16];
	uint32_t sizes[16];
    BLPRGBAColor palette[256];
} BLPHeader;

struct Texture {
	Texture(GLuint _id, char* _key) : id(_id), key(_key), retainCount(1) {};
	
	char* key;
	GLuint id;
	int retainCount;
};

#pragma pack(pop)

extern std::map<char*, Texture*, bool(*)(char*, char*)> gTextures;

Texture* TextureFromFile(const char* file);
Texture* TextureFromPNGFile(const char* file);

Texture* TextureFromMPQFile(const char* file);
Texture* TextureFromBLPData(char* data, size_t size, char* key);

void TextureRelease(Texture* texture);
Texture* TextureRetain(Texture* texture);

#endif