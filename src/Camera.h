#ifndef CAMERA_H
#define CAMERA_H

#include "vector.h"

class Camera {
	public:
		Camera();
		void init();
		void calculateProjection();
		void calculateModelView();
		void calculateProjectionAndModelView();
		
		void turnUp(double radians);
		void turnRight(double radians);

		Vector3d forward;
		Vector3d up;
		Vector3d right;
		Vector3d position;

		double near;
		double far;
		double fieldOfView;
		double viewHeight;
		double aspectRatio;		
		bool isOrthogonal;

		double projection[16];
		double modelView[16];
};

#endif