#ifndef CONTROLS_H
#define CONTROLS_H

class Camera;

enum ControlKey {
	ControlKeyForward,
	ControlKeyBackward,
	ControlKeyLeft,
	ControlKeyRight,
	ControlKeyUp,
	ControlKeyDown,
	ControlKeyCount,
};

void ControlsInit();
void ControlKeyPressed(ControlKey key);
void ControlKeyReleased(ControlKey key);

void UpdateCamera(Camera* camera, long long int usec);

#endif