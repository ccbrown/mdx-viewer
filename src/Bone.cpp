#include "Bone.h"

#include "MDX.h"
#include "Render.h"

Bone::Bone() {
	mpos.loadIdentity();
	mrot.loadIdentity();
}

Bone::~Bone() {
}

bool Bone::initWithMDXData(MDXBone* bone, char* data, size_t size) {
	_parent = bone->parent;
	_pivotPoint = bone->pivotPoint;
	
	if (!_translation.initWithMDXData(bone->translation, data, size)) {
		printf("Couldn't initialize animations.\n");
		return false;
	}

	if (!_rotation.initWithMDXData(bone->rotation, data, size)) {
		printf("Couldn't initialize animations.\n");
		return false;
	}
	
	if (!_scale.initWithMDXData(bone->scale, data, size)) {
		printf("Couldn't initialize animations.\n");
		return false;
	}

	return true;
}

void Bone::update(Bone* bones, unsigned int boneCount, uint32_t animation, uint32_t time) {
	if (_animation = animation && _time == time) {
		return;
	}
	
	_animation = animation;
	_time = time;

	// update matrices
	Matrix4x4f m;
	m.loadTranslation(_pivotPoint);
	
	if (_translation.isAnimated(_animation)) {
		Vector3f t = _translation.getValue(_animation, _time);
		m.translate(t);
	}
	
	if (_rotation.isAnimated(_animation)) {
		Vector4f r = _rotation.getValue(_animation, _time);
		m.quaternionRotate(r);
	}

	if (_scale.isAnimated(_animation)) {
		Vector3f s = _scale.getValue(_animation, _time);
		m.scale(s);
	}
	
	Vector3f np = _pivotPoint * -1.0;
	m.translate(np);
	
	if (_parent >= 0 && _parent < boneCount) {
		bones[_parent].update(bones, boneCount, animation, time);
		mpos = bones[_parent].mpos * m;
	} else {
		mpos = m;
	}
	
	// TODO: Update mrot.
}

void Bone::draw(Bone* bones, unsigned int boneCount) {
	UseRenderProgram(RenderProgramBasic);
	glPointSize(2);

	glBegin(GL_POINTS);
	glColor4f(1.0, 0.0, 0.0, 1.0);
	Vector3f adjustedv = mpos * _pivotPoint;
	glVertex3fv((GLfloat*)&adjustedv);
	glEnd();

	if (_parent >= 0 && _parent < boneCount) {
		glBegin(GL_LINES);
		glColor4f(0.0, 1.0, 1.0, 1.0);
		Vector3f adjustedpv = bones[_parent].mpos * bones[_parent]._pivotPoint;
		glVertex3fv((GLfloat*)&adjustedv);
		glVertex3fv((GLfloat*)&adjustedpv);
		glEnd();
	}
}