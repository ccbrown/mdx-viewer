#ifndef ADT_H
#define ADT_H

#include "Buffer.h"
#include "Vector.h"
#include "WMO.h"

#include <stdint.h>
#include <list>

struct ADTMDXPlacement {
	uint32_t mdx;
	uint32_t uniqueId;
	Vector3f position;
	Vector3f rotation;
	uint16_t scale;
	uint16_t flags;
};

struct ADTWMOPlacement {
	uint32_t wmo;
	uint32_t uniqueId;
	Vector3f position;
	Vector3f rotation;
	Vector3f lowerBounds;
	Vector3f upperBounds;
	uint16_t flags;
	uint16_t doodadSet;
	uint16_t nameSet;
	uint16_t unused;
};

struct ADTChunkTextureLayer {
	uint32_t texture;
	uint32_t flags;
	uint32_t mcalOffset;
	int32_t  effectId;
};

struct ADTChunkHeader {
	uint32_t flags;
	uint32_t indexX;
	uint32_t indexY;
	uint32_t layers;
	uint32_t doodadReferences;
	uint32_t heightsOffset;
	uint32_t normalsOffset;
	uint32_t layerOffset;
	uint32_t referencesOffset;
	uint32_t alphasOffset;
	uint32_t alphaSize;
	uint32_t shadowsOffset;
	uint32_t shadowSize;
	uint32_t areaId;
	uint32_t mapObjectReferences;
	uint32_t holes;
	char     lowQualityTextureMap[16];
	uint32_t predTex;
	uint32_t noEffectDoodad;
	uint32_t soundEmittersOffset;
	uint32_t soundEmitters;
	uint32_t liquidsOffset;
	uint32_t liquidsSize;
	Vector3f position;
	uint32_t mccvOffset;
	uint32_t mclvOffset;
	uint32_t unused; 
};

class ADT;

class ADTChunk {
	public:
	
		ADTChunk();
		~ADTChunk();
	
		bool init(ADT* adt, const char* data, size_t size);

		bool parse(const char* data, size_t size);
		
		void draw();
		
		void destroy();	
	
	private:
	
		ADT* _adt;
	
		ADTChunkHeader _header;
		
		Buffer<ADTChunkTextureLayer> _textureLayers;
	
		float _vertexHeights[145];
};

class ADT {

	public:
	
		ADT();
		~ADT();
				
		bool open(const char* name);

		bool parse(bool isBase, const char* name);
		
		unsigned int textures();
		Texture* texture(unsigned int n);
		
		void animate(long long unsigned int time);
		
		void draw();
		
		void close();

	private:
		
		Buffer<char> _name;

		std::list<WMO*> _wmos;
		std::list<MDX*> _models;
		
		ADTChunk _chunks[256];
		
		std::vector<Texture*> _textures;
};

#endif