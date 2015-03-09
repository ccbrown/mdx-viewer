#ifndef VECTOR_H
#define VECTOR_H

#import <math.h>

struct Vector2f {
	float x;
	float y;
};

struct Vector3f {
	float x;
	float y;
	float z;

	Vector3f() : x(0.0), y(0.0), z(0.0) {};
	Vector3f(const float f) : x(f), y(f), z(f) {};
	Vector3f(const float _x, const float _y, const float _z) : x(_x), y(_y), z(_z) {};
	Vector3f operator+(const Vector3f &right);
	Vector3f& operator+=(const Vector3f &right);
	Vector3f operator-(const Vector3f &right);
	Vector3f& operator-=(const Vector3f &right);
	Vector3f operator*(const float scalar);
	Vector3f operator/(const float invscalar);
};

struct Vector3d {
	double x;
	double y;
	double z;

	Vector3d() : x(0.0), y(0.0), z(0.0) {};
	Vector3d(const double _x, const double _y, const double _z) : x(_x), y(_y), z(_z) {};
	Vector3d operator+(const Vector3d &right);
	Vector3d& operator+=(const Vector3d &right);
	Vector3d operator-(const Vector3d &right);
	Vector3d& operator-=(const Vector3d &right);
	Vector3d operator*(const double scalar);
	Vector3d& operator*=(const double scalar);
	Vector3d operator/(const double invscalar);
	Vector3d& operator/=(const double invscalar);
	double operator*(const Vector3d &right); // Dot product
};

struct Vector4s {
	short x;
	short y;
	short z;
	short w;

	Vector4s() : x(0), y(0), z(0), w(0)  {};
	Vector4s(const short _x, const short _y, const short _z, const short _w) : x(_x), y(_y), z(_z), w(_w) {};
};

struct Vector4f {
	float x;
	float y;
	float z;
	float w;

	Vector4f() : x(0.0), y(0.0), z(0.0), w(0.0) {};
	Vector4f(const float _x, const float _y, const float _z, const float _w) : x(_x), y(_y), z(_z), w(_w) {};
	Vector4f operator+(const Vector4f &right);
	Vector4f& operator+=(const Vector4f &right);
	Vector4f operator-(const Vector4f &right);
	Vector4f& operator-=(const Vector4f &right);
	Vector4f operator*(const float scalar);
	Vector4f operator/(const float invscalar);
};

double VectorMagnitude(Vector3d vector);
Vector3d VectorCrossProduct(Vector3d a, Vector3d b);
Vector3d VectorNormalize(Vector3d vector);

#endif