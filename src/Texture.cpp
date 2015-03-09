#include "Texture.h"
#include "Image.h"
#include "Utility.h"
#include "MPQ.h"
#include "Render.h"
#include "main.h";

bool TextureKeyComp(char* lhs, char* rhs) {
	return strcmp(lhs, rhs) < 0;
}

std::map<char*, Texture*, bool(*)(char*, char*)> gTextures(TextureKeyComp);

Texture* TextureFromFile(const char* file) {
	char* ext = strrchr(file, '.');
	if (!strcmp(ext, ".png")) {
		return TextureFromPNGFile(file);
	} else {
		return NULL;
	}
}

Texture* TextureFromPNGFile(const char* file) {
	char* key = FileKey(file);
	
	if (key == NULL) {
		return NULL;
	}
	
	Texture* texture = gTextures[key];
	
	if (texture != NULL) {
		free(key);
		return TextureRetain(texture);
	}
		
	int width, height;
	bool hasAlpha;
	GLubyte* data;

	if (!LoadPNGImage((char*)file, width, height, hasAlpha, &data)) {
		free(key);
		return NULL;
	}

	GLuint id;

	gOpenGLMutex.lock();
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, (hasAlpha ? 4 : 3), width, height, 0, (hasAlpha ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, data);
	gOpenGLMutex.unlock();

	free(data);	

	return (gTextures[key] = new Texture(id, key));
}

Texture* TextureFromMPQFile(const char* file) {
	char* key = FileKey(file);
	
	if (key == NULL) {
		return NULL;
	}
	
	Texture* texture = gTextures[key];
	
	if (texture != NULL) {
		free(key);
		return TextureRetain(texture);
	}
		
	MPQFile tex(file);
	
	if (!tex.data()) {
		free(key);
		return NULL;
	}

	char* ext = strrchr(file, '.');
	if (!strcmp(ext, ".blp") || !strcmp(ext, ".BLP")) {
		texture = TextureFromBLPData(tex.data(), tex.size(), key);
	} else {
		printf("Unknown texture extension.\n");
	}
	
	if (texture) {
		return texture;
	}
	
	free(key);
	return NULL;
}
	
Texture* TextureFromBLPData(char* data, size_t size, char* key) {
	if (size < sizeof(BLPHeader)) {
		printf("Couldn't read BLP header.\n");
		return NULL;
	}
	
	BLPHeader* header = (BLPHeader*)data;
	
	if (header->magic != '2PLB') {
		printf("Invalid BLP magic.\n");
		return NULL;
	}
		
	GLuint id;

	if (header->type == BLP_TYPE_DIRECTX) {
		if (header->encoding == BLP_ENCODING_DIRECTX) {
			GLint format;
			int blockSize;
			
			switch (header->alphaEncoding) {
				case BLP_ALPHA_ENCODING_DXT2_3:
					format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
					blockSize = 16;
					break;
				case BLP_ALPHA_ENCODING_DXT4_5:
					format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
					blockSize = 16;
					break;
				default:
					format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
					blockSize = 8;
			}
					
			int maxmips = header->hasMips ? 16 : 1;
			int width  = header->width;
			int height = header->height;
		
			gOpenGLMutex.lock();
			glGenTextures(1, &id);
			glBindTexture(GL_TEXTURE_2D, id);

			for (int i = 0; i < maxmips; ++i) {
				if (!header->offsets[i] || !header->sizes[i]) {
					break;
				}
				if (width == 0) {
					width = 1;
				}
				if (height == 0) {
					height = 1;
				}
				size_t mipsize = ((width + 3) / 4) * ((height + 3) / 4) * blockSize;
				if (mipsize != header->sizes[i] || header->offsets[i] + mipsize > size) {
					printf("Couldn't create texture.\n");
					gOpenGLMutex.unlock();
					return NULL;
				}
				glCompressedTexImage2DARB(GL_TEXTURE_2D, i, format, width, height, 0, mipsize, data + header->offsets[i]);
				width >>= 1;
				height >>= 1;
			}

			if (header->hasMips) {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			} else {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
			
			gOpenGLMutex.unlock();
		} else {
			printf("Unsupported encoding.\n");
			return NULL;
		}
	} else {
		printf("Unsupported BLP type.\n");
		return NULL;
	}

	return (gTextures[key] = new Texture(id, key));
}

void TextureRelease(Texture* texture) {
	if (texture != NULL && --texture->retainCount <= 0) {
		gOpenGLMutex.lock();
		gTextures.erase(texture->key);
		glDeleteTextures(1, &texture->id);
		gOpenGLMutex.unlock();
		free(texture->key);
		delete texture;
	}
}

Texture* TextureRetain(Texture* texture) {
	if (texture != NULL) {
		++texture->retainCount;
	}
	return texture;
}