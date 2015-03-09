#ifndef BONE_H
#define BONE_H

#include <stdlib.h>
#include <vector>

#include "Animation.h"
#include "MDXStructs.h"
#include "matrix.h"

class Bone {

	public:
	
		Bone();
		~Bone();

		Matrix4x4f mpos;
		Matrix4x4f mrot;

		bool initWithMDXData(MDXBone* bone, char* data, size_t size);
		
		void update(Bone* bones, unsigned int boneCount, uint32_t animation, uint32_t time);
		
		void draw(Bone* bones, unsigned int boneCount);

	private:
		int _parent;
		Vector3f _pivotPoint;
		
		Animation<Vector3f> _translation;
		Animation<Vector4f, Vector4s, Animation4sTo4f> _rotation;
		Animation<Vector3f> _scale;
		
		uint32_t _animation;
		uint32_t _time;
};

#endif