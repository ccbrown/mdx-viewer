#include "DBC.h"
#include "MDX.h"
#include "MPQ.h"
#include "Render.h"
#include "Utility.h"

#include <stdio.h>
#include <string.h>
#include <netinet/in.h>

MDX::MDX() {
	_skin1          = NULL;
	_skin2          = NULL;
	_skin3          = NULL;

	_name           = NULL;

	for (int i = 0; i < MDX_MAX_TEXTURES; ++i) {
		_textures[i] = NULL;
	}
}

MDX::~MDX() {
	close();
}

bool MDX::open(const char* name) {
	// NOTE: If this is ever compiled on a platform with a different byte order, this function will need some work.
	// NOTE: The mallocs could also use some error checking just in case.

	if (!name) {
		printf("Invalid name.\n");
		return false;
	}
		
	free(_name);
	_name = (char*)malloc(strlen(name) + 5); // give it a little bit of extra room for adding numbers / extensions
	strcpy(_name, name);

	size_t nl = strlen(name);

	// TODO: is an mdl an mdx too?
	bool isMDX = (!strcmp(_name + nl - 4, ".mdx") || !strcmp(_name + nl - 4, ".MDX")); // as opposed to an m2

	if (nl < 5 || (!isMDX && strcmp(_name + nl - 3, ".m2") && strcmp(_name + nl - 3, ".M2"))) {
		printf("Invalid name.\n");
		return false;
	}

	if (isMDX) {
		strcpy(_name + nl - 4, ".m2");
	}
	
	MPQFile f(_name);
	char* data  = f.data();
	size_t size = f.size();

	if (isMDX) {
		strcpy(_name + nl - 4, ".mdx");
	}

	if (!data) {
		return false;
	}

	_position    = Vector3f(0.0, 0.0, 0.0);
	_orientation = Vector4f(0.0, 0.0, 0.0, 0.0);
	_scale       = Vector3f(1.0, 1.0, 1.0);

	_parentTransformation = NULL;
	
	_animation = -1;

	updateTransformation();
	
	if (size < sizeof(M2Header)) {
		printf("Couldn't read M2 header.\n");
		return false;
	}
	
	memcpy(&_header, data, sizeof(M2Header));
		
	if (_header.magic != '02DM') {
		printf("Invalid magic.\n");
		return false;
	}

	if (_header.views < 1 || _header.views > 100) {
		printf("Invalid views.\n");
		return false;
	}
	
	if (size < _header.verticesOffset + sizeof(MDXVertex) * _header.vertices) {
		printf("Couldn't read vertices.\n");
		return false;
	}
	
	_vertices.set(data + _header.verticesOffset, sizeof(MDXVertex) * _header.vertices);
		
	if (size < _header.texturesOffset + sizeof(MDXTexture) * _header.textures || _header.textures > MDX_MAX_TEXTURES) {
		printf("Couldn't read textures.\n");
		return false;
	}
	
	MDXTexture* t = (MDXTexture*)(data + _header.texturesOffset);

	char* lastSlash = strrchr(_name, '\\');
	size_t dirLength = (lastSlash ? lastSlash - _name + 1 : 0);
	Buffer<char> skinNameBuffer;
	if (_skin1 || _skin2 || _skin3) {
		size_t largestSkinName = (_skin1 ? strlen(_skin1) : 0);
		if (_skin2 && strlen(_skin2) > largestSkinName) {
			largestSkinName = strlen(_skin2);
		}
		if (_skin3 && strlen(_skin3) > largestSkinName) {
			largestSkinName = strlen(_skin3);
		}
		skinNameBuffer.alloc(dirLength + largestSkinName + 4 + 1); // the +4 is for ".blp"
		skinNameBuffer.write(0, _name, dirLength);
	}
	char* skinName = *skinNameBuffer;
	
	for (unsigned int i = 0; i < _header.textures; ++i, ++t) {
		char* name = data + t->nameOffset;
		if (size < t->nameOffset + t->nameLength || name[t->nameLength - 1] != '\0') {
			printf("Couldn't read texture.\n");
			return false;
		}
		
		char* textureName = NULL;
		
		switch (t->type) {
			case MDXTextureTypeName:
				textureName = name;
				break;
			case MDXTextureTypeSkin1:
				if (!_skin1) {
					printf("Unknown skin 1.\n");
					return false;
				}
				sprintf(skinName + dirLength, "%s.blp", _skin1);
				textureName = skinName;
				break;
			case MDXTextureTypeSkin2:
				if (!_skin2) {
					printf("Unknown skin 2.\n");
					return false;
				}
				sprintf(skinName + dirLength, "%s.blp", _skin2);
				textureName = skinName;
				break;
			case MDXTextureTypeSkin3:
				if (!_skin3) {
					printf("Unknown skin 3.\n");
					return false;
				}
				sprintf(skinName + dirLength, "%s.blp", _skin3);
				textureName = skinName;
				break;
			default:
				printf("Unknown texture type %u.\n", t->type);
				return false;
		}
		
		printf("%s\n", textureName);
		if (!textureName || !(_textures[i] = TextureFromMPQFile(textureName))) {
			printf("Couldn't load texture.\n");
			return false;
		}
	}

	if (_header.textureLookupsOffset + sizeof(uint16_t) * _header.textureLookups > size) {
		printf("Couldn't read texture lookups. %x\n", _header.textureLookupsOffset);
		return false;
	}
	
	_textureLookups.set(data + _header.textureLookupsOffset, sizeof(uint16_t) * _header.textureLookups);

	if (size < _header.renderSettingsOffset + sizeof(MDXRenderSetting) * _header.renderSettings) {
		printf("Couldn't read render settings.\n");
		return false;
	}
	
	_renderSettings.set(data + _header.renderSettingsOffset, sizeof(MDXRenderSetting) * _header.renderSettings);
	
	if (size < _header.bonesOffset + sizeof(MDXBone) * _header.bones) {
		printf("Couldn't read bones.\n");
		return false;
	}

	_bones.construct(_header.bones);

	MDXBone* b = (MDXBone*)(data + _header.bonesOffset);
	for (unsigned int i = 0; i < _header.bones; ++i, ++b) {
		if (!_bones[i].initWithMDXData(b, data, size)) {
			printf("Couldn't initialize bones.\n");
			return false;
		}
	}
		
	if (size < _header.animationsOffset + sizeof(MDXAnimation) * _header.animations) {
		printf("Couldn't read animations.\n");
		return false;
	}
	
	_animations.set(data + _header.animationsOffset, sizeof(MDXAnimation) * _header.animations);
	
	// this will print out information for each animation in the model
//	MDXAnimation* a = *_animations;
//	DBC<DBCAnimationDataRecord>* adbc = DBCAnimationData();
//	for (unsigned int i = 0; i < _header.animations; ++i, ++a) {
//		printf("%u: %.2f, %u @ %u, %d - ", i, a->moveSpeed, a->length, a->playSpeed, a->nextAnimation);
//		DBCAnimationDataRecord* r = adbc->recordWithId(a->animationId);
//		if (!r) {
//			printf("Animation not found.\n");
//			return false;
//		}
//		const char* aname = adbc->string(r->name);
//		if (aname) {
//			printf("%s\n", aname);
//		} else {
//			printf("[unknown]\n");
//		}
//	}

	// color animations

	if (size < _header.colorAnimationsOffset + sizeof(MDXAnimationBlock) * 2 * _header.colorAnimations) {
		printf("Couldn't read color animations.\n");
		return false;
	}
	
	_colorAnimations.construct(_header.colorAnimations);

	MDXColorAnimation* ca  = *_colorAnimations;
	MDXAnimationBlock* cab = (MDXAnimationBlock*)(data + _header.colorAnimationsOffset);
	for (unsigned int i = 0; i < _header.colorAnimations; ++i, ++ca) {
		ca->rgb.initWithMDXData(*cab++, data, size);
		ca->a.initWithMDXData(*cab++, data, size);
	}	
	
	// transparency animations

	if (size < _header.transparencyAnimationsOffset + sizeof(MDXAnimationBlock) * _header.transparencyAnimations) {
		printf("Couldn't read transparency animations.\n");
		return false;
	}

	if (size < _header.transparencyAnimationLookupsOffset + sizeof(int16_t) * _header.transparencyAnimationLookups) {
		printf("Couldn't read transparency animation lookups.\n");
		return false;
	}

	_transparencyAnimations.construct(_header.transparencyAnimations);

	MDXTransparencyAnimation* tra = *_transparencyAnimations;
	MDXAnimationBlock* trab = (MDXAnimationBlock*)(data + _header.transparencyAnimationsOffset);
	for (unsigned int i = 0; i < _header.transparencyAnimations; ++i, ++tra, ++trab) {
		tra->a.initWithMDXData(*trab, data, size);
	}	

	_transparencyAnimationLookups.set(data + _header.transparencyAnimationLookupsOffset, sizeof(int16_t) * _header.transparencyAnimationLookups);
	
	for (unsigned int i = 0; i < _transparencyAnimationLookups.count(); ++i) {
		if (_transparencyAnimationLookups[i] >= 0 && _transparencyAnimationLookups[i] >= _header.transparencyAnimations) {
			printf("Couldn't read transparency animation lookups.\n");
			return false;
		}
	}
	
	// texture animations
		
	if (size < _header.textureAnimationsOffset + sizeof(MDXAnimationBlockSet) * _header.textureAnimations) {
		printf("Couldn't read texture animations.\n");
		return false;
	}

	if (size < _header.textureAnimationLookupsOffset + sizeof(int16_t) * _header.textureAnimationLookups) {
		printf("Couldn't read texture animation lookups.\n");
		return false;
	}

	_textureAnimations.construct(_header.textureAnimations);
	
	MDXTextureAnimation* ta = *_textureAnimations;
	MDXAnimationBlockSet* ab = (MDXAnimationBlockSet*)(data + _header.textureAnimationsOffset);
	for (unsigned int i = 0; i < _header.textureAnimations; ++i, ++ta, ++ab) {
		ta->translation.initWithMDXData(ab->translation, data, size);
		ta->rotation.initWithMDXData(ab->rotation, data, size);
		ta->scale.initWithMDXData(ab->scale, data, size);
	}

	_textureAnimationLookups.set(data + _header.textureAnimationLookupsOffset, sizeof(int16_t) * _header.textureAnimationLookups);
	
	for (unsigned int i = 0; i < _textureAnimationLookups.count(); ++i) {
		if (_textureAnimationLookups[i] >= 0 && _textureAnimationLookups[i] >= _header.textureAnimations) {
			printf("Couldn't read texture animation lookups.\n");
			return false;
		}
	}
	
	// Done reading the file. Initialize stuff.
		
	_vertexBuffer.alloc(3 * sizeof(GLfloat) * _header.vertices);
	_normalBuffer.alloc(3 * sizeof(GLfloat) * _header.vertices);

	for (unsigned int i = 0; i < _header.vertices; ++i) {
		_vertexBuffer[i * 3 + 0] = _vertices[i].position.x;
		_vertexBuffer[i * 3 + 1] = _vertices[i].position.y;
		_vertexBuffer[i * 3 + 2] = _vertices[i].position.z;

		_normalBuffer[i * 3 + 0] = _vertices[i].normal.x;
		_normalBuffer[i * 3 + 1] = _vertices[i].normal.y;
		_normalBuffer[i * 3 + 2] = _vertices[i].normal.z;
	}

	setAnimation(0); // just get the model doing something if possible (temporary?)

	return setLevelOfDetail(_header.views - 1);
}

bool MDX::openCreatureDisplayId(unsigned int id) {
	DBCCreatureDisplayInfoRecord* r = DBCCreatureDisplayInfo()->recordWithId(id);
	if (!r) {
		printf("Creature not found.\n");
		return false;
	}
	
	DBCCreatureModelDataRecord* r2 = DBCCreatureModelData()->recordWithId(r->model);
	if (!r2) {
		printf("Model not found.\n");
		return false;
	}

	setSkins(DBCCreatureDisplayInfo()->string(r->skin1), 
	         DBCCreatureDisplayInfo()->string(r->skin2), 
	         DBCCreatureDisplayInfo()->string(r->skin3)
	         );

	return open(DBCCreatureModelData()->string(r2->path));
}

void MDX::setSkins(const char* skin1, const char* skin2, const char* skin3) {	
	if (skin1) {
		free(_skin1);
		_skin1 = (char*)malloc(strlen(skin1) + 1);
		strcpy(_skin1, skin1);
	} else {
		_skin1 = NULL;
	}

	if (skin2) {
		free(_skin2);
		_skin2 = (char*)malloc(strlen(skin2) + 1);
		strcpy(_skin2, skin2);
	} else {
		_skin2 = NULL;
	}

	if (skin3) {
		free(_skin3);
		_skin3 = (char*)malloc(strlen(skin3) + 1);
		strcpy(_skin3, skin3);
	} else {
		_skin3 = NULL;
	}
}

bool MDX::setLevelOfDetail(int level) {
	// NOTE: If this is ever compiled on a platform with a different byte order, this function will need some work.
	// NOTE: The mallocs could also use some error checking just in case.

	if (level > 99) {
		printf("Invalid level of detail.\n");
		return false;
	}
	
	size_t nl = strlen(_name);

	bool isMDX = (!strcmp(_name + nl - 4, ".mdx") || !strcmp(_name + nl - 4, ".MDX")); // as opposed to an m2

	sprintf(_name + nl - (isMDX ? 4 : 3), "%02d.skin", level);
	
	MPQFile f(_name);
	char* data  = f.data();
	size_t size = f.size();

	strcpy(_name + nl - (isMDX ? 4 : 3), ".mdx");
	
	if (!data) {
		return false;
	}

	if (size < sizeof(SKINHeader)) {
		printf("Couldn't read SKIN header.\n");
		return false;
	}

	memcpy(&_skinHeader, data, sizeof(SKINHeader));
		
	if (_skinHeader.magic != 'NIKS') {
		printf("Invalid magic.\n");
		return false;
	}
		
	if (_skinHeader.triangleIndices % 3 != 0) {
		printf("Invalid triangle vertices.\n");
		return false;
	}

	if (_skinHeader.indicesOffset + sizeof(GLushort) * _skinHeader.indices > size) {
		printf("Couldn't read indices.\n");
		return false;
	}
	
	if (_skinHeader.triangleIndicesOffset + sizeof(GLushort) * _skinHeader.triangleIndices > size) {
		printf("Couldn't read triangles.\n");
		return false;
	}
	
	_indexBuffer.alloc(sizeof(GLushort) * _skinHeader.triangleIndices);
	
	uint16_t* indices   = (uint16_t*)(data + _skinHeader.indicesOffset);
	uint16_t* triangles = (uint16_t*)(data + _skinHeader.triangleIndicesOffset);
		
	for (unsigned int i = 0; i < _skinHeader.triangleIndices; ++i) {
		uint16_t t = triangles[i];
		if (t >= _skinHeader.indices) {
			printf("Invalid index.\n");
			return false;
		}
		uint16_t n = indices[t];
		if (n >= _header.vertices) {
			printf("Invalid index.\n");
			return false;
		}
		_indexBuffer[i] = n;
	}	
	
	if (_skinHeader.geosetsOffset + sizeof(SKINGeoset) * _skinHeader.geosets > size) {
		printf("Couldn't read geosets.\n");
		return false;
	}

	if (_skinHeader.materialsOffset + sizeof(SKINMaterial) * _skinHeader.materials > size) {
		printf("Couldn't read materials.\n");
		return false;
	}

	_geosets.alloc(sizeof(MDXGeoset) * _skinHeader.geosets);

	SKINMaterial* materials = (SKINMaterial*)(data + _skinHeader.materialsOffset);

	SKINGeoset* g = (SKINGeoset*)(data + _skinHeader.geosetsOffset);
	for (unsigned int i = 0; i < _skinHeader.geosets; ++i, ++g) {		
		if (g->startTriangleIndex + g->triangleIndices > _skinHeader.triangleIndices) {
			printf("Couldn't read geoset.\n");
			return false;
		}

		MDXGeoset* gs = &_geosets[i];
		gs->startTriangleIndex = g->startTriangleIndex;
		gs->triangleIndices    = g->triangleIndices;

		SKINMaterial* m = materials;
		for (unsigned int j = 0; ; ++j, ++m) {
			if (j >= _skinHeader.materials) {
				printf("Couldn't find material for geoset.\n");
				return false;
			}
			if (m->texture >= _header.textureLookups || _textureLookups[m->texture] >= _header.textures || m->animation >= _header.textureAnimationLookups) {
				printf("Couldn't read material.\n");
				return false;
			}
			if (m->geoset == i) {
				gs->texture1              = _textureLookups[m->texture];
				gs->texture1Animation     = _textureAnimationLookups[m->animation];
				gs->transparencyAnimation = _transparencyAnimationLookups[m->transparency];
				gs->colorAnimation        = m->color;
				if (m->renderSetting >= _renderSettings.count()) {
					printf("Couldn't read render settings.\n");
					return false;
				}
				gs->renderFlags = _renderSettings[m->renderSetting].flags;
				gs->shadingMode = _renderSettings[m->renderSetting].shadingMode;
				// TODO: Multitexturing.
				break;
			}
		}
	}
	
	return true;
}

void MDX::setAnimation(int animation) {
	setAnimation(animation, 0);
}

void MDX::setAnimation(int animation, unsigned long long int origin) {
	_animation = animation;
	_animationOrigin = origin;
}

void MDX::setPosition(Vector3f position) {
	_position = position;

	updateTransformation();
}

void MDX::setOrientation(Vector4f orientation) {
	_orientation = orientation;

	updateTransformation();
}

void MDX::setScale(Vector3f scale) {
	_scale = scale;
	
	updateTransformation();
}

void MDX::setParentTransformation(Matrix4x4f* transformation) {
	_parentTransformation = transformation;

	updateTransformation();
}

void MDX::updateTransformation() {
	if (_parentTransformation) {
		_transformation = *_parentTransformation;
	} else {
		_transformation.loadIdentity();
	}

	_transformation.translate(_position);
	_transformation.quaternionRotate(_orientation);
	_transformation.scale(_scale);
}

void MDX::animate(long long unsigned int time) {
	if (_animation < 0 || _animation >= _header.animations || time < _animationOrigin) {
		return;
	}

	long long unsigned int animationTime = time - _animationOrigin;
	int nextAnimation = _animations[_animation].nextAnimation;
	
	// advance to the next animation if necessary
	while (nextAnimation >= 0 && nextAnimation < _header.animations) {
		if (animationTime <= _animations[_animation].length) {
			break;
		}
		_animation = nextAnimation;
		_animationOrigin += _animations[_animation].length;
		animationTime -= _animations[_animation].length;
		nextAnimation = _animations[_animation].nextAnimation;
	}
	
	// update the bones
	for (unsigned int i = 0; i < _header.bones; ++i) {
		_bones[i].update(*_bones, _header.bones, _animation, animationTime);
	}

	MDXVertex* v = *_vertices;
	for (unsigned int i = 0; i < _header.vertices; ++i, ++v) {
		Vector3f position(0.0, 0.0, 0.0);
		Vector3f normal(0.0, 0.0, 0.0);

		for (int b = 0; b < 4; ++b) {
			if (v->boneWeights[b] > 0) {
				position += _bones[v->boneIndices[b]].mpos * v->position * v->boneWeights[b] / 255.0;
				normal   += _bones[v->boneIndices[b]].mrot * v->normal   * v->boneWeights[b] / 255.0;
			}
		}

		_vertexBuffer[i * 3 + 0] = position.x;
		_vertexBuffer[i * 3 + 1] = position.y;
		_vertexBuffer[i * 3 + 2] = position.z;

		_normalBuffer[i * 3 + 0] = normal.x;
		_normalBuffer[i * 3 + 1] = normal.y;
		_normalBuffer[i * 3 + 2] = normal.z;
	}

	// update the color animations
	for (int i = 0; i < _header.colorAnimations; ++i) {
		Vector3f rgb = _colorAnimations[i].rgb.getValue(_animation, animationTime);
		float a = _colorAnimations[i].a.getValue(_animation, animationTime);
		_colorAnimations[i].color = Vector4f(rgb.x, rgb.y, rgb.z, a);
	}
		
	// update the transparency animations
	for (int i = 0; i < _header.transparencyAnimations; ++i) {
		_transparencyAnimations[i].alpha = _transparencyAnimations[i].a.getValue(_animation, animationTime);
	}
	
	// update the texture animations
	MDXTextureAnimation* ta = *_textureAnimations;
	for (int i = 0; i < _header.textureAnimations; ++i, ++ta) {
		ta->mat.loadIdentity();

		if (ta->translation.isAnimated(_animation)) {
			Vector3f t = ta->translation.getValue(_animation, animationTime);
			ta->mat.translate(t);
		}
		
		// TODO: Figure out how the rotation here is actually supposed to work (or is it working right?).
		if (ta->rotation.isAnimated(_animation)) {
			Vector4f r = ta->rotation.getValue(_animation, animationTime);
			ta->mat.quaternionRotate(r);
		}
	
		if (ta->scale.isAnimated(_animation)) {
			Vector3f s = ta->scale.getValue(_animation, animationTime);
			ta->mat.scale(s);
		}
	}
}

void MDX::draw(long long unsigned int time) {
	animate(time);
	draw();
}
	
void MDX::draw() {
	glPushMatrix();
	glMultMatrixf(_transformation.m);

	UseRenderProgram(RenderProgramMDX);

	MDXGeoset* gs = *_geosets;
	for (unsigned int i = 0; i < _skinHeader.geosets; ++i, ++gs) {
		SetRenderProgramUniformi("shadingMode", gs->shadingMode);
		SetRenderProgramUniformf("alpha", _transparencyAnimations[gs->transparencyAnimation].alpha);
		SetRenderProgramUniformf("color", _colorAnimations[gs->colorAnimation].color.x, _colorAnimations[gs->colorAnimation].color.y, _colorAnimations[gs->colorAnimation].color.z, _colorAnimations[gs->colorAnimation].color.w);

		if (gs->renderFlags & MDX_RENDER_FLAG_TWO_SIDED) {
			glDisable(GL_CULL_FACE);
		} else {
			glEnable(GL_CULL_FACE);
		}

		if (gs->renderFlags & MDX_RENDER_FLAG_TRANSPARENT) {
			glDepthMask(GL_FALSE);
		}

		glBindTexture(GL_TEXTURE_2D, _textures[gs->texture1]->id);
	
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		
		if (gs->texture1Animation >= 0) {
			glMatrixMode(GL_TEXTURE);
			glPushMatrix();

			glMultMatrixf(_textureAnimations[gs->texture1Animation].mat.m);
		}

		glNormalPointer(GL_FLOAT, 0, *_normalBuffer);
		glTexCoordPointer(2, GL_FLOAT, sizeof(MDXVertex), &_vertices[0].textureCoord);
		glVertexPointer(3, GL_FLOAT, 0, *_vertexBuffer);
		glDrawElements(GL_TRIANGLES, gs->triangleIndices, GL_UNSIGNED_SHORT, &_indexBuffer[gs->startTriangleIndex]);

		if (gs->texture1Animation >= 0) {
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
		}

		glDepthMask(GL_TRUE);
	}

	glPopMatrix();
}

void MDX::drawBones() {
	glPushMatrix();
	glMultMatrixf(_transformation.m);

	for (unsigned int i = 0; i < _header.bones; ++i) {
		_bones[i].draw(*_bones, _header.bones);
	}

	glPopMatrix();
}

void MDX::close() {
	for (int i = 0; i < MDX_MAX_TEXTURES; ++i) {
		TextureRelease(_textures[i]);
		_textures[i] = NULL;
	}

	free(_name);
	_name = NULL;

	free(_skin1);
	_skin1 = NULL;
	
	free(_skin2);
	_skin2 = NULL;
	
	free(_skin3);
	_skin3 = NULL;
}