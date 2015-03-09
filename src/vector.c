#include "vector.h"

// Vector3f

Vector3f Vector3f::operator+(const Vector3f &right) {
	return Vector3f(x + right.x, y + right.y, z + right.z);
}

Vector3f& Vector3f::operator+=(const Vector3f &right) {
	x += right.x;
	y += right.y;
	z += right.z;
	return *this;
}

Vector3f Vector3f::operator-(const Vector3f &right) {
	return Vector3f(x - right.x, y - right.y, z - right.z);
}

Vector3f& Vector3f::operator-=(const Vector3f &right) {
	x -= right.x;
	y -= right.y;
	z -= right.z;
	return *this;
}

Vector3f Vector3f::operator*(const float scalar) {
	return Vector3f(x * scalar, y * scalar, z * scalar);
}

Vector3f Vector3f::operator/(const float invscalar) {
	return Vector3f(x / invscalar, y / invscalar, z / invscalar);
}

// Vector3d

Vector3d Vector3d::operator+(const Vector3d &right) {
	return Vector3d(x + right.x, y + right.y, z + right.z);
}

Vector3d& Vector3d::operator+=(const Vector3d &right) {
	x += right.x;
	y += right.y;
	z += right.z;
	return *this;
}

Vector3d Vector3d::operator-(const Vector3d &right) {
	return Vector3d(x - right.x, y - right.y, z - right.z);
}

Vector3d& Vector3d::operator-=(const Vector3d &right) {
	x -= right.x;
	y -= right.y;
	z -= right.z;
	return *this;
}

Vector3d Vector3d::operator*(const double scalar) {
	return Vector3d(x * scalar, y * scalar, z * scalar);
}

Vector3d& Vector3d::operator*=(const double scalar) {
	x *= scalar;
	y *= scalar;
	z *= scalar;
	return *this;
}

// Vector4f

Vector4f Vector4f::operator+(const Vector4f &right) {
	return Vector4f(x + right.x, y + right.y, z + right.z, w + right.w);
}

Vector4f& Vector4f::operator+=(const Vector4f &right) {
	x += right.x;
	y += right.y;
	z += right.z;
	w += right.w;
	return *this;
}

Vector4f Vector4f::operator-(const Vector4f &right) {
	return Vector4f(x - right.x, y - right.y, z - right.z, w - right.w);
}

Vector4f& Vector4f::operator-=(const Vector4f &right) {
	x -= right.x;
	y -= right.y;
	z -= right.z;
	w -= right.w;
	return *this;
}

Vector4f Vector4f::operator*(const float scalar) {
	return Vector4f(x * scalar, y * scalar, z * scalar, w * scalar);
}

Vector4f Vector4f::operator/(const float invscalar) {
	return Vector4f(x / invscalar, y / invscalar, z / invscalar, w / invscalar);
}

// Dot product
double Vector3d::operator*(const Vector3d &right) {
	return x * right.x + y * right.y + z * right.z;
}

Vector3d Vector3d::operator/(const double invscalar) {
	return Vector3d(x / invscalar, y / invscalar, z / invscalar);
}

Vector3d& Vector3d::operator/=(const double invscalar) {
	x /= invscalar;
	y /= invscalar;
	z /= invscalar;
	return *this;
}

double VectorMagnitude(Vector3d vector) {
	return sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
}

Vector3d VectorCrossProduct(Vector3d a, Vector3d b) {
	return Vector3d(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

Vector3d VectorNormalize(Vector3d vector) {
	Vector3d normalized;
	double denominator = VectorMagnitude(vector);
	normalized.x = vector.x / denominator;
	normalized.y = vector.y / denominator;
	normalized.z = vector.z / denominator;
	return normalized;
}
