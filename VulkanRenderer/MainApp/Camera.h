#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glfw3.h>
#include <glfw3native.h>

struct Camera
{
	glm::vec3 position = glm::vec3(3.0f, 0.0f, 0.0f);
	glm::vec3 rotation;
	glm::quat orientation;
	glm::quat invOrient = glm::conjugate(orientation);

	glm::vec3 forward = orientation * glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 cameraUp = orientation * glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 right = glm::normalize(glm::cross(forward, cameraUp));

	glm::vec3 worldUp = glm::vec3(0.0f, 0.0f, 1.0f);

	//glm::mat4 transform;
	glm::mat4 view;

	float yaw = 90.0f;
	float pitch = 250.0f;
	float roll = 0.0f;
	float fov = 70.0f;
	float minFov = 1.0f;
	float maxFov = 120.0f;
	float zoomScale = 1.0f;

	float sensitivity = 0.5f;

	void updatePositon(int key, float speed);
	void updateCameraRotation(float pitch, float yaw, float roll);
	void updateFOV(double yOffset);
	void zoomCamera(double yOffset);

	void updateView(float dt);
};


#endif // ifndef CAMERA_H
