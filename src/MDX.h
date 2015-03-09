#ifndef MDX_H
#define MDX_H

#include <OpenGL/gl.h>
#include <stdlib.h>

#include "Animation.h"
#include "Buffer.h"
#include "Bone.h"
#include "MDXStructs.h"
#include "Texture.h"

#define MDX_MAX_TEXTURES 32

#define MDX_RENDER_FLAG_TWO_SIDED   0x04
#define MDX_RENDER_FLAG_TRANSPARENT 0x10

struct M2Header {
	uint32_t magic;
	uint32_t version;
	uint32_t nameLength;
	uint32_t nameOffset;
	uint32_t flags;

	uint32_t globalSequences;
	uint32_t globalSequencesOffset;
	
	uint32_t animations;
	uint32_t animationsOffset;
	uint32_t animationLookups;
	uint32_t animationLookupsOffset;

	uint32_t bones;
	uint32_t bonesOffset;
	uint32_t keyBoneLookup;
	uint32_t keyBoneLookupOffset;

	uint32_t vertices;
	uint32_t verticesOffset;
	uint32_t views;
	// views are stored in .skins

	uint32_t colorAnimations;
	uint32_t colorAnimationsOffset;
	uint32_t textures;
	uint32_t texturesOffset;
	uint32_t transparencyAnimations;
	uint32_t transparencyAnimationsOffset;
	uint32_t textureAnimations;
	uint32_t textureAnimationsOffset;
	uint32_t textureReplacements;
	uint32_t textureReplacementsOffset;

	uint32_t renderSettings;
	uint32_t renderSettingsOffset;
	
	uint32_t boneLookup;
	uint32_t boneLookupOffset;

	uint32_t textureLookups;
	uint32_t textureLookupsOffset;
	uint32_t textureUnitLookups;
	uint32_t textureUnitLookupsOffset;
	uint32_t transparencyAnimationLookups;
	uint32_t transparencyAnimationLookupsOffset;
	uint32_t textureAnimationLookups;
	uint32_t textureAnimationLookupsOffset;

	MDXSphere collisionSphere;
	MDXSphere boundSphere;

	uint32_t boundingTriangles;
	uint32_t boundingTrianglesOffset;
	uint32_t boundingVertices;
	uint32_t boundingVerticesOffset;
	uint32_t boundingNormals;
	uint32_t boundingNormalsOffset;

	uint32_t attachments;
	uint32_t attachmentsOffset;
	uint32_t attachmentLookup;
	uint32_t attachmentLookupOffset;
	
	uint32_t events;
	uint32_t eventsOffset;
	
	uint32_t lights;
	uint32_t lightsOffset;
	
	uint32_t cameras;
	uint32_t camerasOffset;
	uint32_t cameraLookup;
	uint32_t cameraLookupOffset;
	
	uint32_t ribbonEmitters;
	uint32_t ribbonEmittersOffset;
	uint32_t particleEmitters;
	uint32_t particleEmittersOffset;
};

struct SKINHeader {
	uint32_t magic;
    uint32_t indices;
    uint32_t indicesOffset;
    uint32_t triangleIndices;
	uint32_t triangleIndicesOffset;
    uint32_t properties;
	uint32_t propertiesOffset;
    uint32_t geosets;
	uint32_t geosetsOffset;
    uint32_t materials;
	uint32_t materialsOffset;
	uint32_t bones;
};

struct SKINGeoset {
	uint32_t part;
	uint16_t startVertex;
	uint16_t vertices;
	uint16_t startTriangleIndex;
	uint16_t triangleIndices;
	uint16_t bones;
	uint16_t startBone;
	uint16_t unknown;
	uint16_t rootBone;
	Vector3f centerOfMass;
	Vector3f boundingBoxCenter;
	float    radius;
};

struct SKINMaterial {
	uint16_t flags;
	uint16_t shading;
	uint16_t geoset;
	uint16_t unknown;
	int16_t  color;
	uint16_t renderSetting;
	uint16_t textureUnit1;
	uint16_t mode;
	uint16_t texture;
	uint16_t textureUnit2;
	uint16_t transparency;
	uint16_t animation;
};

struct MDXGeoset {
	uint16_t startTriangleIndex;
	uint16_t triangleIndices;
	uint16_t texture1;
	int16_t  texture1Animation;
	int16_t  transparencyAnimation;
	int16_t  colorAnimation;
	uint16_t renderFlags;
	uint16_t shadingMode;
};

struct MDXTextureAnimation {
	Animation<Vector3f> translation;
	Animation<Vector4f, Vector4s, Animation4sTo4f> rotation;
	Animation<Vector3f> scale;
	Matrix4x4f mat;
};

struct MDXColorAnimation {
	Animation<Vector3f> rgb;
	Animation<float, int16_t, Animation1sTo1f> a;
	Vector4f color;
};

struct MDXTransparencyAnimation {
	Animation<float, int16_t, Animation1sTo1f> a;
	float alpha;
};

class MDX {

	public:
	
		MDX();
		~MDX();
				
		bool open(const char* name);
		bool openCreatureDisplayId(unsigned int id);
				
		void setSkins(const char* skin1, const char* skin2, const char* skin3);
		bool setLevelOfDetail(int level);
		
		void setAnimation(int animation);
		void setAnimation(int animation, long long unsigned int origin);

		void setPosition(Vector3f position);
		void setOrientation(Vector4f orientation);
		void setScale(Vector3f scale);
		void setParentTransformation(Matrix4x4f* transformation);
		void updateTransformation();

		void animate(long long unsigned int time);

		void draw(long long unsigned int time);
		
		void draw();
		void drawBones();
		
		void close();

	private:
		
		char* _name;

		char* _skin1;
		char* _skin2;
		char* _skin3;

		M2Header   _header;
		Buffer<MDXVertex> _vertices;
		
		SKINHeader _skinHeader;

		Buffer<GLfloat>  _vertexBuffer;
		Buffer<GLfloat>  _normalBuffer;
		Buffer<GLushort> _indexBuffer;

		Texture* _textures[MDX_MAX_TEXTURES];
		Buffer<uint16_t> _textureLookups;
		
		Buffer<MDXAnimation> _animations;
		
		Buffer<MDXGeoset> _geosets;
		
		int _animation;
		long long unsigned int _animationOrigin;

		Buffer<MDXRenderSetting> _renderSettings;

		Buffer<MDXColorAnimation> _colorAnimations;

		Buffer<MDXTransparencyAnimation> _transparencyAnimations;
		Buffer<int16_t> _transparencyAnimationLookups;

		Buffer<MDXTextureAnimation> _textureAnimations;
		Buffer<int16_t> _textureAnimationLookups;
		
		Buffer<Bone> _bones;
	
		Vector3f _position;
		Vector4f _orientation;
		Vector3f _scale;
		
		Matrix4x4f* _parentTransformation;
		Matrix4x4f  _transformation;
};

#endif