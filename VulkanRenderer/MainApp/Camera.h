#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glfw3.h>
#include <glfw3native.h>

struct Camera
{
	glm::mat4 transform;
	glm::vec3 position = glm::vec3(-3.0f, 0.0f, 0.5f);
	glm::vec3 rotation;

	glm::vec3 forward = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 right = glm::normalize(glm::cross(forward, up));

	float yaw = -90.0f;
	float pitch = 0.0f;
	float fov = 45.0f;
	float minFov = 1.0f;
	float maxFov = 120.0f;

	void updatePositon(int key, float speed);
	void updateFOV(double yOffset);
};


#endif // ifndef CAMERA_H
