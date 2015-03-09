#include "Controls.h"
#include "Camera.h"

int gControlStates[ControlKeyCount];

void ControlsInit() {
	for (int i = 0; i < ControlKeyCount; ++i) {
		gControlStates[i] = 0;
	}
}

void ControlKeyPressed(ControlKey key) {
	++gControlStates[key];
}

void ControlKeyReleased(ControlKey key) {
	if (gControlStates[key] > 0) {
		--gControlStates[key];
	}
}

void UpdateCamera(Camera* camera, long long int usec) {
	const double moveSpeed = 0.00008 * usec;
	if (gControlStates[ControlKeyLeft] > 0) {
		camera->position.x -= moveSpeed * camera->right.x;
		camera->position.y -= moveSpeed * camera->right.y;
		camera->position.z -= moveSpeed * camera->right.z;
	}
	if (gControlStates[ControlKeyRight] > 0) {
		camera->position.x += moveSpeed * camera->right.x;
		camera->position.y += moveSpeed * camera->right.y;
		camera->position.z += moveSpeed * camera->right.z;
	}
	if (gControlStates[ControlKeyForward] > 0) {
		camera->position.x += moveSpeed * camera->forward.x;
		camera->position.y += moveSpeed * camera->forward.y;
		camera->position.z += moveSpeed * camera->forward.z;
	}
	if (gControlStates[ControlKeyBackward] > 0) {
		camera->position.x -= moveSpeed * camera->forward.x;
		camera->position.y -= moveSpeed * camera->forward.y;
		camera->position.z -= moveSpeed * camera->forward.z;
	}
	if (gControlStates[ControlKeyUp] > 0) {
		camera->position.x += moveSpeed * camera->up.x;
		camera->position.y += moveSpeed * camera->up.y;
		camera->position.z += moveSpeed * camera->up.z;
	}
	if (gControlStates[ControlKeyDown] > 0) {
		camera->position.x -= moveSpeed * camera->up.x;
		camera->position.y -= moveSpeed * camera->up.y;
		camera->position.z -= moveSpeed * camera->up.z;
	}
	camera->calculateModelView();
}