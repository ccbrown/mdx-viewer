#include "Camera.h"

#include "matrix.h"

Camera::Camera() {
	init();
}

void Camera::init() {
	forward  = Vector3d(0.0, 0.0, -1.0);
	up       = Vector3d(0.0, 1.0, 0.0);
	right    = Vector3d(1.0, 0.0, 0.0);
	position = Vector3d(0.0, 0.0, 0.0);

	near         = 0.01;
	far          = 5000.0;
	fieldOfView  = M_PI / 3.0;
	viewHeight   = 4.0;
	aspectRatio  = 1.0;
	isOrthogonal = false;

	calculateProjectionAndModelView();
}

void Camera::calculateProjection() {
	if (isOrthogonal) {
		mat4d_LoadOrtho(-viewHeight * 0.5 * aspectRatio, viewHeight * 0.5 * aspectRatio, -viewHeight * 0.5, viewHeight * 0.5, near, far, projection);
	} else {
		mat4d_LoadPerspective(fieldOfView, aspectRatio, near, far, projection);
	}
}

void Camera::calculateModelView() {
	double translation[16];
	Vector3d neg = position * -1.0;
	mat4d_LoadTranslation(&neg.x, translation);
	mat4d_LoadLookAt(&forward.x, &up.x, &right.x, modelView);
	mat4d_MultiplyMat4d(modelView, translation, modelView);
}

void Camera::calculateProjectionAndModelView() {
	calculateProjection();
	calculateModelView();
}

void Camera::turnUp(double radians) {
	forward = VectorNormalize(forward * cos(radians) + up * sin(radians));
	up = VectorCrossProduct(right, forward);
}

void Camera::turnRight(double radians) {
	forward = VectorNormalize(forward * cos(radians) + right * sin(radians));
	right = VectorCrossProduct(forward, up);
}