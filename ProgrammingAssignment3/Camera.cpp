#include "pch.h"
#include "Camera.h"

Camera::Camera() {
	state = FRONT;

	cameraPosition = vec3(0, 0, 10);
	cameraTarget = vec3(0, 0, 0);
	cameraDirection = normalize(cameraPosition - cameraTarget);
	cameraFront = vec3(0, 0, -1);

	vec3 up = vec3(0, 1, 0);
	cameraRight = normalize(cross(up, cameraDirection));
	cameraUp = cross(cameraDirection, cameraRight);

	//view = lookAt(cameraPosition, -cameraDirection, cameraUp);
	step = 1;
}

Camera::~Camera() {

}

void Camera::ViewFront() {
	state = FRONT;
	cameraPosition = vec3(0, 0, 10);
	cameraTarget = vec3(0, 0, 0);
	cameraDirection = normalize(cameraPosition - cameraTarget);
	cameraFront = vec3(0, 0, -1);

	vec3 up = vec3(0, 1, 0);
	cameraRight = normalize(cross(up, cameraDirection));
	cameraUp = cross(cameraDirection, cameraRight);

	//view = lookAt(cameraPosition, -cameraDirection, cameraUp);
}

void Camera::ViewBack() {
	state = FRONT;
	cameraPosition = vec3(0, 0, -10);
	cameraTarget = vec3(0, 0, 0);
	cameraDirection = normalize(cameraPosition - cameraTarget);
	cameraFront = vec3(0, 0, 1);

	vec3 up = vec3(0, 1, 0);
	cameraRight = normalize(cross(up, cameraDirection));
	cameraUp = cross(cameraDirection, cameraRight);

	//view = lookAt(cameraPosition, -cameraDirection, cameraUp);
}

void Camera::ViewRight() {
	state = RIGHT;
	cameraPosition = vec3(-10, 0, 0);
	cameraTarget = vec3(0, 0, 0);
	cameraDirection = normalize(cameraPosition - cameraTarget);
	cameraFront = vec3(1, 0, 0);

	vec3 up = vec3(0, 1, 0);
	cameraRight = normalize(cross(up, cameraDirection));
	cameraUp = cross(cameraDirection, cameraRight);

	//view = lookAt(cameraPosition, -cameraDirection, cameraUp);
}

void Camera::ViewLeft() {
	state = LEFT;
	cameraPosition = vec3(10, 0, 0);
	cameraTarget = vec3(0, 0, 0);
	cameraDirection = normalize(cameraPosition - cameraTarget);
	cameraFront = vec3(-1, 0, 0);

	vec3 up = vec3(0, 1, 0);
	cameraRight = normalize(cross(up, cameraDirection));
	cameraUp = cross(cameraDirection, cameraRight);

	//view = lookAt(cameraPosition, -cameraDirection, cameraUp);
}

void Camera::ViewUp() {
	state = UP;
	cameraPosition = vec3(0, 10, 0);
	cameraTarget = vec3(0, 0, 0);
	cameraDirection = normalize(cameraPosition - cameraTarget);
	cameraFront = vec3(0, -1, 0);

	vec3 up = vec3(0, 0, -1);
	cameraRight = normalize(cross(up, cameraDirection));
	cameraUp = cross(cameraDirection, cameraRight);

	//view = lookAt(cameraPosition, -cameraDirection, cameraUp);
}

void Camera::ZoomIn() {
	switch (state) {
	case FRONT:
		cameraPosition.z -= step;
		break;

	case UP:
		cameraPosition.y -= step;
		break;

	case RIGHT:
		cameraPosition.x += step;
		break;

	case LEFT:
		cameraPosition.x -= step;
		break;

	default:
		break;
	}
}

void Camera::ZoomOut() {
	switch (state) {
	case FRONT:
		cameraPosition.z += step;
		break;

	case UP:
		cameraPosition.y += step;
		break;

	case RIGHT:
		cameraPosition.x -= step;
		break;

	case LEFT:
		cameraPosition.x += step;
		break;

	default:
		break;
	}
}

void Camera::MoveUp() {
	switch (state) {
	case FRONT:
		cameraPosition.y += step;
		break;
	case UP :
		cameraPosition.z -= step;
		break;
	case RIGHT :
		cameraPosition.y += step;
		break;
	case LEFT :
		cameraPosition.y += step;
		break;
	default :
		break;
	}
}

void Camera::MoveRight() {
	switch (state) {
	case FRONT:
		cameraPosition.x += step;
		break;
	case UP :
		cameraPosition.x += step;
		break;
	case RIGHT :
		cameraPosition.z += step;
		break;
	case LEFT :
		cameraPosition.z -= step;
		break;
	default :
		break;
	}

}

void Camera::MoveLeft() {
	switch (state) {
	case FRONT:
		cameraPosition.x -= step;
		break;
	case UP :
		cameraPosition.x -= step;
		break;
	case RIGHT :
		cameraPosition.z -= step;
		break;
	case LEFT :
		cameraPosition.z += step;
		break;
	default :
		break;
	}

}

void Camera::MoveDown() {
	switch (state) {
	case FRONT:
		cameraPosition.y -= step;
		break;
	case UP :
		cameraPosition.z -= step;
		break;
	case RIGHT :
		cameraPosition.y -= step;
		break;
	case LEFT :
		cameraPosition.z += step;
		break;
	default :
		break;
	}

}
