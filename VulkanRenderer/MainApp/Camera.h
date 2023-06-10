#ifndef CAMERA_H
#define CAMERA_H

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glfw3.h>
#include <glfw3native.h>

struct Camera
{
	void updatePositon(int key, float speed);
	void updateCameraRotation(float pitch, float yaw, float roll);
	void updateFOV(double yOffset);
	void zoomCamera(double yOffset);

	void updateModel(float dt);

	void setPerspectiveProjection(float fov, float aspectRatio, float near, float far);

	glm::mat4 getTransform()
	{
		glm::mat4 rot = glm::toMat4(orientation);

		return glm::translate(glm::mat4(1.0f), position) * rot * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
	}

	glm::vec3 position = glm::vec3(0.0f, 3.0f, 0.0f);
	glm::vec3 rotation{};
	glm::quat orientation{};
	glm::quat invOrient = glm::conjugate(orientation);

	glm::vec3 forward = orientation * glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 up = orientation * glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 right = glm::normalize(glm::cross(forward, up));

	glm::vec3 worldUp = glm::vec3(0.0f, 0.0f, 1.0f);

	glm::mat4 invModel;
	glm::mat4 view;
	glm::mat4 invView;
	glm::mat4 proj;

	float yaw = 0.0f;
	float pitch = 250.0f;
	float roll = 0.0f;
	float fov = 70.0f;
	float minFov = 1.0f;
	float maxFov = 120.0f;
	float zoomScale = 1.0f;

	float sensitivity = 0.5f;
	float smoothing = 0.00001f;

	bool canScroll = true;
};


#endif // ifndef CAMERA_H
