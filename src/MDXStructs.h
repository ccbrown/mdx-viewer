#ifndef MDX_STRUCTS_H
#define MDX_STRUCTS_H

#include <stdint.h>

#include "vector.h"

struct MDXSphere {
	Vector3f min;
	Vector3f max;
	float radius;
};

struct MDXVertex {
	Vector3f position;
	uint8_t boneWeights[4];
	uint8_t boneIndices[4];
	Vector3f normal;
	Vector2f textureCoord;
	Vector2f unknown;
};

typedef enum {
	MDXTextureTypeName = 0,
	MDXTextureTypeSkin1 = 11,
	MDXTextureTypeSkin2 = 12,
	MDXTextureTypeSkin3 = 13,
} MDXTextureType;

struct MDXTexture {
	uint32_t type;
	uint32_t flags;
	uint32_t nameLength;
	uint32_t nameOffset;
};

struct MDXTimelineTimes {
	uint32_t times;
	uint32_t timesOffset;
};

struct MDXTimelineKeys {
	uint32_t keys;
	uint32_t keysOffset;
};

struct MDXAnimationBlock {
	uint16_t interpolationType; // 0 is none, 1 is linear, 2 is hermite
	int16_t  sequence;          // global sequence or -1
	uint32_t times;
	uint32_t timesOffset;
	uint32_t keys;
	uint32_t keysOffset;
};

struct MDXBone {
	int32_t keybone; // -1 if this isn't a key bone.
	int32_t flags;   // 8 is billboarded, 512 is transformed
	int16_t parent;
	int16_t geoset;
	int32_t unknown;
	MDXAnimationBlock translation;
	MDXAnimationBlock rotation;
	MDXAnimationBlock scale;
	Vector3f pivotPoint;
};

struct MDXRenderSetting {
	uint16_t flags;
	uint16_t shadingMode;
};

struct MDXAnimation {
	uint16_t animationId;
	uint16_t subanimationId;
	uint32_t length;         // bones seem to be able to have animations of different lengths
	                         // so this may only used for switching to the next animation
	float moveSpeed;
	uint32_t flags;
	uint16_t probability;
	uint16_t unused;
	uint32_t unknown1;
	uint32_t unknown2;
	uint32_t playSpeed;
	MDXSphere boundsSphere;
	int16_t nextAnimation;
	int16_t index;	
};

struct MDXAnimationBlockSet {
	MDXAnimationBlock translation;
	MDXAnimationBlock rotation;
	MDXAnimationBlock scale;
};

#endif