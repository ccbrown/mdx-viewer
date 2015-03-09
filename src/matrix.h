#ifndef MATRIX_H
#define MATRIX_H

#include "vector.h"

struct Matrix4x4f {
	float m[16];
	
	Vector3f operator*(const Vector3f& right);
	Matrix4x4f operator*(const Matrix4x4f& right);
	Matrix4x4f& operator*=(const Matrix4x4f& right);
	
	void loadIdentity();
	
	void loadTranslation(Vector3f& t);
	void loadQuaternionRotation(Vector4f& r);
	void loadScale(Vector3f& s);
	
	void translate(Vector3f& t);
	void quaternionRotate(Vector4f& r);
	void scale(Vector3f& s);
};

// phasing these out in favor of the above structure

void mat4d_LoadTranslation(double* t, double* mout);
void mat4d_LoadLookAt(double *f, double* u, double* r, double* mout);
void mat4d_LoadPerspective(double fov_radians, double aspect, double zNear, double zFar, double* mout);
void mat4d_LoadOrtho(double left, double right, double bottom, double top, double near, double far, double* mout);
void mat4d_MultiplyMat4d(double* a, double* b, double* mout);

#endif