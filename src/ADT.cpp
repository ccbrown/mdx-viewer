#include "ADT.h"
#include "MPQ.h"
#include "Render.h"

#include <stdio.h>
#include <string.h>

ADTChunk::ADTChunk() {
}

ADTChunk::~ADTChunk() {
	destroy();
}

bool ADTChunk::init(ADT* adt, const char* data, size_t size) {
	destroy();
	
	if (size < sizeof(ADTChunkHeader)) {
		printf("Couldn't read chunk header.\n");
		return false;
	}
	
	_adt = adt;

	memcpy(&_header, data, sizeof(ADTChunkHeader));
	
	return parse(data + sizeof(ADTChunkHeader), size - sizeof(ADTChunkHeader));
}
	
bool ADTChunk::parse(const char* data, size_t size) {
	const char* dataptr = data;
	size_t dataRemaining = size;
	
	uint32_t tag;
	uint32_t tagsize;
	while (dataRemaining >= 8) {
		tag  = *(int*)dataptr;
		dataptr += 4;
		tagsize = *(int*)dataptr;
		dataptr += 4;
		
		dataRemaining -= 8;
		
		if (tagsize > dataRemaining) {
			printf("Invalid tag size.\n");
			return false;
		}

		switch (tag) {
			case 'MCVT': { // vertex heights
				if (tagsize != 145 * sizeof(float)) {
					printf("Couldn't read vertex heights.\n");
					return false;
				}
				memcpy(_vertexHeights, dataptr, tagsize);
				break;
			}

			case 'MCLY': { // texture layers (in texX files)
				if (tagsize % sizeof(ADTChunkTextureLayer)) {
					printf("Couldn't read texture layers.\n");
					return false;
				}
				_textureLayers.set(dataptr, tagsize);
				ADTChunkTextureLayer* tl = (ADTChunkTextureLayer*)dataptr;
				for (int i = 0; i < tagsize / sizeof(ADTChunkTextureLayer); ++i, ++tl) {
					if (tl->texture >= _adt->textures()) {
						printf("Couldn't read texture layers.\n");
						return false;
					}
				}
				break;
			}
						
			default:
				// unprogrammed tag
				//printf("Unknown ADT chunk tag: '%c%c%c%c'\n", ((char*)&tag)[3], ((char*)&tag)[2], ((char*)&tag)[1], ((char*)&tag)[0]);
				break;
		}
		
		dataptr += tagsize;
		dataRemaining -= tagsize;
	}

	return true;
}

void ADTChunk::draw() {	
	glPushMatrix();
	glTranslatef(-_header.position.y, -_header.position.x, _header.position.z);
	glScalef(100.0 / 24.0, 100.0 / 24.0, 1.0);
	
	if (_textureLayers.count() > 0) {
		UseRenderProgram(RenderProgramTextured);
		
		// TODO: texture layers
		// TODO: fix texture coordinates

		Texture* t = _adt->texture(_textureLayers[0].texture);

		if (t) {
			glBindTexture(GL_TEXTURE_2D, t->id);
		}
	} else {
		UseRenderProgram(RenderProgramBasic);
		glColor4f(0.0, 1.0, 0.5, 1.0);
	}
	
	// TODO: This stuff should go in a VBO	

	int off = 9;
	float x, y;
	for (y = 0; y < 8; ++y, off += 9) {
	    for (x = 0; x < 8; ++x, ++off) {
	        glBegin(GL_TRIANGLE_FAN);
	        	glTexCoord2f(x / 8.0, y / 8.0);
	            glVertex3f(x, y, _vertexHeights[off]);
	        	glTexCoord2f((x - 0.5f) / 8.0, (y - 0.5f) / 8.0);
	            glVertex3f(x - 0.5f, y - 0.5f, _vertexHeights[off - 9]);
	        	glTexCoord2f((x + 0.5f) / 8.0, (y - 0.5f) / 8.0);
	            glVertex3f(x + 0.5f, y - 0.5f, _vertexHeights[off - 8]);
	        	glTexCoord2f((x + 0.5f) / 8.0, (y + 0.5f) / 8.0);
	            glVertex3f(x + 0.5f, y + 0.5f, _vertexHeights[off + 9]);
	        	glTexCoord2f((x - 0.5f) / 8.0, (y + 0.5f) / 8.0);
	            glVertex3f(x - 0.5f, y + 0.5f, _vertexHeights[off + 8]);
	        	glTexCoord2f((x - 0.5f) / 8.0, (y - 0.5f) / 8.0);
	            glVertex3f(x - 0.5f, y - 0.5f, _vertexHeights[off - 9]);
	        glEnd();
	    }
	}
	
	// TODO: Draw higher resolution terrain
	
	glPopMatrix();
}
	
void ADTChunk::destroy() {
}

ADT::ADT() {
}

ADT::~ADT() {
	close();
}

bool ADT::open(const char* name) {
	close();

	if (!name) {
		printf("Invalid name.\n");
		return false;
	}

	size_t nl = strlen(name);
	if (nl < 5 || (strcmp(name + nl - 4, ".adt") && strcmp(name + nl - 4, ".ADT"))) {
		printf("Invalid name.\n");
		return false;
	}
	
	_name.set(name, strlen(name) + 6); // add room for "_tex0" or "_obj0"
		
	// base adt
	if (!parse(true, *_name)) {
		return false;
	}

	// texture0 adt
	sprintf(_name + nl - 4, "_tex0.adt");
	if (!parse(false, *_name)) {
		return false;
	}

	// object adt
	sprintf(_name + nl - 4, "_obj0.adt");
	if (!parse(false, *_name)) {
		return false;
	}

	sprintf(_name + nl - 4, ".adt");

	return true;
}

bool ADT::parse(bool isBase, const char* name) {
	// NOTE: If this is ever compiled on a platform with a different byte order, this function will need some work.
	// NOTE: The mallocs could also use some error checking just in case.
	
	MPQFile f(name);

	if (!f.data()) {
		return false;
	}

	char* dataptr         = f.data();
	size_t dataRemaining  = f.size();

	uint32_t* mwid        = NULL;
	uint32_t  mwidEntries = 0;
	
	char* mwmo            = NULL;
	uint32_t mwmoLength   = 0;

	uint32_t* mmid        = NULL;
	uint32_t  mmidEntries = 0;
	
	char* mmdx            = NULL;
	uint32_t mmdxLength   = 0;
	
	unsigned int chunks   = 0;

	uint32_t tag;
	uint32_t tagsize;
	while (dataRemaining >= 8) {
		tag  = *(int*)dataptr;
		dataptr += 4;
		tagsize = *(int*)dataptr;
		dataptr += 4;
		
		dataRemaining -= 8;
		
		if (tagsize > dataRemaining) {
			printf("Invalid tag size.\n");
			return false;
		}
		
		switch (tag) {

			case 'MTEX': { // texture filenames
				if (tagsize == 0) {
					break;
				}
				if (dataptr[tagsize - 1] != '\0') {
					printf("Couldn't read texture filenames.\n");
					return false;
				}
				char* tname = dataptr;
				while (*tname) {
					_textures.push_back(TextureFromMPQFile(tname));
					while (*tname != '\0') {
						++tname;
					}
					while (*tname == '\0' && tname - dataptr + 1 < tagsize) {
						++tname;
					}
				}
				break;
			}

			case 'MMDX': { // mdx filenames
				if (tagsize == 0) {
					break;
				}
				mmdx = (char*)dataptr;
				mmdxLength = tagsize;
				if (mmdx[mmdxLength - 1] != '\0') {
					printf("Couldn't read MDX filenames.\n");
					return false;
				}
				break;
			}

			case 'MMID': { // mdx filename offsets
				mmid = (uint32_t*)dataptr;
				mmidEntries = tagsize / sizeof(uint32_t);
				break;
			}
			
			case 'MWMO': { // wmo filenames
				if (tagsize == 0) {
					break;
				}
				mwmo = (char*)dataptr;
				mwmoLength = tagsize;
				if (mwmo[mwmoLength - 1] != '\0') {
					printf("Couldn't read WMO filenames.\n");
					return false;
				}
				break;
			}

			case 'MWID': { // wmo filename offsets
				mwid = (uint32_t*)dataptr;
				mwidEntries = tagsize / sizeof(uint32_t);
				break;
			}

			case 'MDDF': { // mdx placement
				if (tagsize == 0) {
					break;
				}
				if (!mmid || !mmdx || tagsize % sizeof(ADTMDXPlacement)) {
					printf("Couldn't read MDX placements.\n");
					return false;
				}
				ADTMDXPlacement* p = (ADTMDXPlacement*)dataptr;
				for (unsigned int i = 0; i < tagsize / sizeof(ADTMDXPlacement); ++i, ++p) {
					if (p->mdx >= mmidEntries || mmid[p->mdx] >= mmdxLength) {
						printf("Couldn't read MDX placement.\n");
						return false;
					}
					MDX* m = new MDX();
					if (!m->open(mmdx + mmid[p->mdx])) {
						printf("Couldn't open MDX.\n");
						delete m;
					} else {
						m->setPosition(Vector3f(p->position.x - 51200.0 / 3.0, p->position.z - 51200.0 / 3.0, p->position.y));
						// TODO: rotation
						m->setScale(Vector3f(p->scale / 1024.0));
						_models.push_back(m);
					}
				}
				break;
			}
						
			case 'MODF': { // wmo placement
				if (tagsize == 0) {
					break;
				}
				if (!mwid || !mwmo || tagsize % sizeof(ADTWMOPlacement)) {
					printf("Couldn't read WMO placements.\n");
					return false;
				}
				ADTWMOPlacement* p = (ADTWMOPlacement*)dataptr;
				for (unsigned int i = 0; i < tagsize / sizeof(ADTWMOPlacement); ++i, ++p) {
					if (p->wmo >= mwidEntries || mwid[p->wmo] >= mwmoLength) {
						printf("Couldn't read WMO placement.\n");
						return false;
					}
					WMO* m = new WMO();
					if (!m->open(mwmo + mwid[p->wmo])) {
						printf("Couldn't open WMO.\n");
						delete m;
						return false;
					}
					m->setPosition(Vector3f(p->position.x - 51200.0 / 3.0, p->position.z - 51200.0 / 3.0, p->position.y));
					// TODO: rotation
					_wmos.push_back(m);
				}
				break;
			}
			
			case 'MCNK': { // chunk
				if (chunks >= 256) {
					printf("Too many chunks in ADT.\n");
					return false;
				}
				if (isBase) {
					if (!_chunks[chunks].init(this, dataptr, tagsize)) {
						printf("Couldn't read ADT chunk.\n");
						return false;
					}
				} else if (!_chunks[chunks].parse(dataptr, tagsize)) {
					printf("Couldn't read ADT chunk.\n");
					return false;
				}
				++chunks;
				break;
			}
						
			default:
				// unprogrammed tag
				//printf("Unknown WMO tag: '%c%c%c%c'\n", ((char*)&tag)[3], ((char*)&tag)[2], ((char*)&tag)[1], ((char*)&tag)[0]);
				break;
		}
		
		dataptr += tagsize;
		dataRemaining -= tagsize;
	}		
	
	if (chunks != 256) {
		printf("Couldn't read ADT chunks.\n");
		return false;
	}
	
	return true;
}

unsigned int ADT::textures() {
	return _textures.size();
}

Texture* ADT::texture(unsigned int n) {
	return _textures[n];
}

void ADT::animate(long long unsigned int time) {
	for (std::list<WMO*>::iterator it = _wmos.begin(); it != _wmos.end(); ++it) {
		(*it)->animate(time);
	}

	for (std::list<MDX*>::iterator it = _models.begin(); it != _models.end(); ++it) {
		(*it)->animate(time);
	}
}

void ADT::draw() {
	for (std::list<WMO*>::iterator it = _wmos.begin(); it != _wmos.end(); ++it) {
		(*it)->draw();
	}

	for (std::list<MDX*>::iterator it = _models.begin(); it != _models.end(); ++it) {
		(*it)->draw();
	}

	for (int i = 0; i < 256; ++i) {
		_chunks[i].draw();
	}
}

void ADT::close() {
	for (std::list<WMO*>::iterator it = _wmos.begin(); it != _wmos.end(); ++it) {
		delete *it;
	}

	for (std::list<MDX*>::iterator it = _models.begin(); it != _models.end(); ++it) {
		delete *it;
	}
	
	for (std::vector<Texture*>::iterator it = _textures.begin(); it != _textures.end(); ++it) {
		TextureRelease(*it);
	}
	_textures.clear();
}
