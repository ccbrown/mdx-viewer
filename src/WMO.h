#ifndef WMO_H
#define WMO_H

#include <stdint.h>
#include <stdlib.h>
#include <list>

#include "Buffer.h"
#include "MDX.h"
#include "vector.h"

struct WMOColor {
	unsigned char red;
	unsigned char green;
	unsigned char blue;
	unsigned char alpha;
};

struct WMOHeader {
	uint32_t textures;   // number of textures
	uint32_t groups;     // number of groups
	uint32_t portals;    // number of portals
	uint32_t lights;     // number of lights
	uint32_t models;     // number of models
	uint32_t doodads;    // number of doodads
	uint32_t doodadSets; // number of doodad sets
	WMOColor color;      // color
	uint32_t id;         // id
	Vector3f bb1;        // bounding box corner 1
	Vector3f bb2;        // bounding box corner 2
	uint32_t liquidType; // liquid type
};

struct WMOMaterial {
	uint32_t flags;
	uint32_t shadingMode;
	uint32_t transparent;  // 0 for opaque, 1 for transparent
	uint32_t texture1;     // offset of a texture name in MOTX
	WMOColor color1;
	uint32_t flags1;
	uint32_t texture2;     // offset of a texture name in MOTX
	WMOColor color2;
	uint32_t flags2;
	uint32_t f1;
	uint32_t f2;
	uint32_t dx[5];
};

struct WMOMaterialInstance {
	uint32_t shadingMode;
	Texture* texture1;
	Texture* texture2;
	
	WMOMaterialInstance() : texture1(NULL), texture2(NULL) {};
	~WMOMaterialInstance() { 
		TextureRelease(texture1); 
		TextureRelease(texture2);
	};
};

struct WMODoodadInstance {
	uint32_t nameOffset;
	Vector3f position;
	Vector4f orientation;
	float    scale;
	WMOColor color;
};

struct WMOGroupInfo {
	uint32_t flags;
	Vector3f bb1;
	Vector3f bb2;
	int32_t  name;
};

struct WMOGroupHeader {
	uint32_t nameOffset;
	uint32_t descOffset;
	uint32_t flags;
	Vector3f bb1;
	Vector3f bb2;
	uint16_t portalIndex;
	uint16_t portals;
	uint16_t batchesA;
	uint16_t batchesB;
	uint32_t batchesC;
	uint8_t  fogIndices[4];
	uint32_t liquidType;
	uint32_t groupId;
	uint32_t zero1;
	uint32_t zero2;
};

struct WMOBatch {
	uint32_t unknown1;
	uint32_t unknown2;
	uint32_t unknown3;
	uint32_t startIndex;
	uint16_t indices;
	uint16_t startVertex;
	uint16_t endVertex;
	uint8_t  zero;
	uint8_t  material;
};

class WMO;

class WMOGroup {
	public:
		
		WMOGroup(WMO* rootwmo);
		~WMOGroup();
		
		bool init(WMOGroupHeader* header, const char* data, size_t size);

		void draw();
		
		void destroy();
		
	private:

		WMO* _rootwmo;

		WMOGroupHeader _header;
		Buffer<WMOBatch> _batches;

		unsigned int _triangles;
		
		Buffer<GLushort> _indexBuffer;
		Buffer<GLfloat>  _vertexBuffer;
		Buffer<GLfloat>  _normalBuffer;
		Buffer<GLfloat>  _texCoordBuffer[2];
};

class WMO {

	public:
	
		WMO();
		WMO(WMO* rootwmo);
		~WMO();
		
		bool open(const char* name);
		
		void animate(long long unsigned int time);
		
		unsigned int materials();
		WMOMaterialInstance* material(unsigned int n);

		void setPosition(Vector3f position);
		void setOrientation(Vector4f orientation);
		void setScale(Vector3f scale);
		void setParentTransformation(Matrix4x4f* transformation);

		void updateTransformation();
		
		void draw();
		
		void close();

	private:
		
		char* _name;
	
		WMOHeader _header;

		Buffer<WMOMaterialInstance> _materials;

		WMO* _rootwmo;
		WMOGroup* _group;

		std::list<WMO*> _groupFiles;
		
		std::list<MDX*> _models;	

		Vector3f _position;
		Vector4f _orientation;
		Vector3f _scale;

		Matrix4x4f* _parentTransformation;
		Matrix4x4f  _transformation;
};

#endif