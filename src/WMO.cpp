#include "WMO.h"

#include "MPQ.h"
#include "Render.h"
#include "Utility.h"

#include <stdio.h>
#include <string.h>
#include <netinet/in.h>

WMOGroup::WMOGroup(WMO* rootwmo) {
	_rootwmo = rootwmo;
	
	_triangles = 0;
}

WMOGroup::~WMOGroup() {
	destroy();
}

bool WMOGroup::init(WMOGroupHeader* header, const char* data, size_t size) {
	destroy();
	
	if (!_rootwmo) {
		printf("No root WMO.\n");
		return false;
	}

	memcpy(&_header, header, sizeof(WMOGroupHeader));

	const char* dataptr = data;
	size_t dataRemaining = size;
	
	unsigned int highestIndex = 0;
	unsigned int vertices     = 0;
	unsigned int normals      = 0;
	unsigned int texCoords[2] = {0};
	
	unsigned int tc = 0;
	
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

			case 'MOVI': { // map object vertex indices
				if (tagsize % (sizeof(GLushort) * 3)) {
					printf("Couldn't read vertex indices.\n");
					return false;
				}
				_triangles = tagsize / (sizeof(GLushort) * 3);
				_indexBuffer.set(dataptr, tagsize);
				for (unsigned int i = 0; i < _triangles * 3; ++i) {
					if (_indexBuffer[i] > highestIndex) {
						highestIndex = _indexBuffer[i];
					}
				}
				break;
			}

			case 'MOVT': { // map object vertices
				if (tagsize % (sizeof(GLfloat) * 3)) {
					printf("Couldn't read vertices.\n");
					return false;
				}
				vertices = tagsize / (sizeof(GLfloat) * 3);
				_vertexBuffer.set(dataptr, tagsize);
				break;
			}

			case 'MONR': { // map object vertex normals
				if (tagsize % (sizeof(GLfloat) * 3)) {
					printf("Couldn't read normals.\n");
					return false;
				}
				normals = tagsize / (sizeof(GLfloat) * 3);
				_normalBuffer.set(dataptr, tagsize);
				break;
			}

			case 'MOTV': { // map object vertex texture coordinates
				if (tc >= 2) {
					printf("Invalid texture coordinate block.\n");
					return false;
				}
				if (tagsize % (sizeof(GLfloat) * 2)) {
					printf("Couldn't read texture coordinates.\n");
					return false;
				}
				texCoords[tc] = tagsize / (sizeof(GLfloat) * 2);
				_texCoordBuffer[tc].set(dataptr, tagsize);
				++tc;
				break;
			}

			case 'MOBA': { // map object batches
				unsigned int batches = tagsize / sizeof(WMOBatch);
				if (_header.batchesA + _header.batchesB + _header.batchesC != batches || tagsize % sizeof(WMOBatch)) {
					printf("Couldn't read batches.\n");
					return false;
				}
				WMOBatch* b = (WMOBatch*)dataptr;
				for (unsigned int i = 0; i < batches; ++i, ++b) {
					if (b->material >= _rootwmo->materials()) {
						printf("Couldn't read batches.\n");
						return false;
					}
				}
				_batches.set(dataptr, tagsize);
				break;
			}			

			default:
				// unprogrammed tag
				//printf("Unknown WMO group tag: '%c%c%c%c'\n", ((char*)&tag)[3], ((char*)&tag)[2], ((char*)&tag)[1], ((char*)&tag)[0]);
				break;
		}
		
		dataptr += tagsize;
		dataRemaining -= tagsize;
	}

	if (vertices != normals || (tc >= 1 && normals != texCoords[0]) || (tc >= 2 && texCoords[0] != texCoords[1]) || (_triangles > 0 && highestIndex >= vertices)) {
		printf("Couldn't read vertex information.\n");
		return false;
	}

	return true;
}

void WMOGroup::draw() {
	if (_triangles > 0) {
		UseRenderProgram(RenderProgramWMO);
		glEnable(GL_CULL_FACE);

		WMOBatch* b = *_batches;
		for (unsigned int i = 0; i < _header.batchesA + _header.batchesB + _header.batchesC; ++i, ++b) {
			WMOMaterialInstance* m = _rootwmo->material(b->material);

			SetRenderProgramUniformi("shadingMode", m->shadingMode);

			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_NORMAL_ARRAY);

			if (m->texture1 && *_texCoordBuffer[0]) {
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glBindTexture(GL_TEXTURE_2D, m->texture1->id);
				glTexCoordPointer(2, GL_FLOAT, 0, *_texCoordBuffer[0]);
			}
			
			if (m->texture2 && *_texCoordBuffer[1]) {
				glActiveTexture(GL_TEXTURE1);
				glClientActiveTexture(GL_TEXTURE1);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glBindTexture(GL_TEXTURE_2D, m->texture2->id);
				glTexCoordPointer(2, GL_FLOAT, 0, *_texCoordBuffer[1]);
				glActiveTexture(GL_TEXTURE0);
				glClientActiveTexture(GL_TEXTURE0);
			}
			
			glNormalPointer(GL_FLOAT, 0, *_normalBuffer);
			glVertexPointer(3, GL_FLOAT, 0, *_vertexBuffer);
			glDrawElements(GL_TRIANGLES, b->indices, GL_UNSIGNED_SHORT, &_indexBuffer[b->startIndex]);
		}
	}
}

void WMOGroup::destroy() {	
	_triangles = 0;
}

WMO::WMO() {
	_name = NULL;
	
	_group = NULL;
	
	_rootwmo = this;
}

WMO::WMO(WMO* rootwmo) {
	_name = NULL;
	
	_group = NULL;
	
	_rootwmo = rootwmo;
}

WMO::~WMO() {
	close();
}

bool WMO::open(const char* name) {
	close();
	
	if (!name) {
		printf("Invalid name.\n");
		return false;
	}
		
	free(_name);
	_name = (char*)malloc(strlen(name) + 5); // give it a little bit of extra room for adding group numbers
	strcpy(_name, name);
		
	MPQFile f(_name);
	
	if (!f.data()) {
		return false;
	}

	_position    = Vector3f(0.0, 0.0, 0.0);
	_orientation = Vector4f(0.0, 0.0, 0.0, 0.0);
	_scale       = Vector3f(1.0, 1.0, 1.0);

	_parentTransformation = NULL;

	char* dataptr = f.data();
	size_t dataRemaining = f.size();
	
	char* modelNames = NULL;
	size_t modelNamesSize = 0;

	char* textureNames = NULL;
	size_t textureNamesSize = 0;
	
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
		
			case 'MVER': { // map version
				if (tagsize < 4) {
					printf("Couldn't read version.\n");
					return false;
				}
				break;
			}
				
			case 'MOHD': { // map object header
				if (tagsize < sizeof(WMOHeader)) {
					printf("Couldn't read header.\n");
					return false;
				}
				memcpy(&_header, dataptr, sizeof(WMOHeader));
				break;
			}
				
			case 'MOTX': { // map object texture names
				if (dataptr[tagsize - 1] != '\0') {
					printf("Couldn't read texture names.\n");
					return false;
				}
				textureNames = dataptr;
				textureNamesSize = tagsize;
				break;
			}

			case 'MOMT': { // map object materials
				if (!textureNames || tagsize != sizeof(WMOMaterial) * _header.textures) {
					printf("Couldn't read materials.\n");
					return false;
				}
				_materials.construct(_header.textures);
				WMOMaterial* m = (WMOMaterial*)dataptr;
				for (unsigned int i = 0; i < _header.textures; ++i, ++m) {
					if (m->texture1 >= textureNamesSize || m->texture2 >= textureNamesSize) {
						printf("Couldn't read materials.\n");
						return false;
					}
					_materials[i].shadingMode = m->shadingMode;
					_materials[i].texture1    = TextureFromMPQFile(textureNames + m->texture1);
					_materials[i].texture2    = TextureFromMPQFile(textureNames + m->texture2);
				}
				break;
			}
			
			case 'MODN': { // map object model names
				if (tagsize == 0) {
					break;
				}
				if (dataptr[tagsize - 1] != '\0') {
					printf("Couldn't read model names.\n");
					return false;
				}
				modelNames = dataptr;
				modelNamesSize = tagsize;
				break;
			}
			
			case 'MODD': { // map object doodad instances
				if (!modelNames || tagsize % sizeof(WMODoodadInstance)) {
					printf("Couldn't read doodad instances.\n");
					return false;
				}
				WMODoodadInstance* di = (WMODoodadInstance*)dataptr;
				for (unsigned int i = 0; i < tagsize / sizeof(WMODoodadInstance); ++i, ++di) {
					if (di->nameOffset >= modelNamesSize) {
						printf("Couldn't read doodad instance.\n");
						return false;
					}
					MDX* m = new MDX();
					if (!m->open(modelNames + di->nameOffset)) {
						printf("WARNING: Couldn't open %s\n", modelNames + di->nameOffset);
						delete m;
					} else {
						m->setPosition(di->position);
						m->setOrientation(di->orientation);
						m->setScale(Vector3f(di->scale, di->scale, di->scale));
						m->setParentTransformation(&_transformation);
						_models.push_back(m);
					}
				}
				break;
			}

			case 'MOGI': { // map object group info
				unsigned int groups = tagsize / sizeof(WMOGroupInfo);
				if (tagsize % sizeof(WMOGroupInfo) || groups > 512) {
					printf("Couldn't read group info.\n");
					return false;
				}
				WMOGroupInfo* g = (WMOGroupInfo*)dataptr;
				size_t nameChangeOffset = strlen(_name) - 4;
				for (unsigned int i = 0; i < groups; ++i, ++g) {
					sprintf(_name + nameChangeOffset, "_%03u.wmo", i);
					WMO* groupWMO = new WMO(_rootwmo);
					if (!groupWMO->open(_name)) {
						printf("Couldn't open %s\n", _name);
						delete groupWMO;
						return false;
					}
					_groupFiles.push_back(groupWMO);
				}
				strcpy(_name + nameChangeOffset, ".wmo");
				break;
			}
			
			case 'MOGP': { // map object group
				if (tagsize < sizeof(WMOGroupHeader)) {
					printf("Couldn't read group header.\n");
					return false;
				}
				delete _group;
				_group = new WMOGroup(_rootwmo);
				if (!_group->init((WMOGroupHeader*)dataptr, dataptr + sizeof(WMOGroupHeader), tagsize - sizeof(WMOGroupHeader))) {
					printf("Couldn't read group.\n");
					return false;
				}
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

	updateTransformation();

	return true;
}

void WMO::animate(long long unsigned int time) {
	for (std::list<WMO*>::iterator it = _groupFiles.begin(); it != _groupFiles.end(); ++it) {
		(*it)->animate(time);
	}

	for (std::list<MDX*>::iterator it = _models.begin(); it != _models.end(); ++it) {
		(*it)->animate(time);
	}
}

unsigned int WMO::materials() {
	return _materials.count();
}

WMOMaterialInstance* WMO::material(unsigned int n) {
	if (n < _header.textures) {
		return &_materials[n];
	}
	return NULL;
}

void WMO::setPosition(Vector3f position) {
	_position = position;

	updateTransformation();
}

void WMO::setOrientation(Vector4f orientation) {
	_orientation = orientation;
	
	updateTransformation();
}

void WMO::setScale(Vector3f scale) {
	_scale = scale;

	updateTransformation();
}

void WMO::setParentTransformation(Matrix4x4f* transformation) {
	_parentTransformation = transformation;

	updateTransformation();
}

void WMO::updateTransformation() {
	if (_parentTransformation) {
		_transformation = *_parentTransformation;
	} else {
		_transformation.loadIdentity();
	}

	_transformation.translate(_position);
	_transformation.quaternionRotate(_orientation);
	_transformation.scale(_scale);

	for (std::list<MDX*>::iterator it = _models.begin(); it != _models.end(); ++it) {
		(*it)->updateTransformation();
	}
}

void WMO::draw() {
	glPushMatrix();
	glMultMatrixf(_transformation.m);
	
	for (std::list<WMO*>::iterator it = _groupFiles.begin(); it != _groupFiles.end(); ++it) {
		(*it)->draw();
	}

	if (_group) {
		_group->draw();
	}

	glPopMatrix();

	for (std::list<MDX*>::iterator it = _models.begin(); it != _models.end(); ++it) {
		(*it)->draw();
		(*it)->drawBones();
	}	
}

void WMO::close() {	
	free(_name);
	_name = NULL;
	
	delete _group;
	_group = NULL;

	for (std::list<WMO*>::iterator it = _groupFiles.begin(); it != _groupFiles.end(); ++it) {
		delete *it;
	}
	_groupFiles.clear();
	
	for (std::list<MDX*>::iterator it = _models.begin(); it != _models.end(); ++it) {
		delete *it;
	}
	_models.clear();
}