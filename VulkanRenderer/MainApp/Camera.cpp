#include "Camera.h"

void Camera::updatePositon(int key, float speed)
{
	switch (key)
	{
	case GLFW_KEY_W:
		position += speed * forward;
		break;
	case GLFW_KEY_S:
		position -= speed * forward;
		break;
	case GLFW_KEY_A:
		position -= right * speed;
		break;
	case GLFW_KEY_D:
		position += right * speed;
		break;
	case GLFW_KEY_E:
		position += speed * up;
		break;
	case GLFW_KEY_Q:
		position -= speed * up;
	}
}

void Camera::updateCameraRotation(float yaw, float pitch, float roll, float dt)
{
	pitch += pitch * sensitivity * dt;
	yaw += yaw * sensitivity * dt;
	//roll += roll * dt;

	if(pitch > 89.0f)
		pitch = 89.0f;
	if(pitch < -89.0f)
		pitch = -89.0f;

	if(yaw > 360.0f || yaw < -360.0f)
		yaw = 0.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.z = sin(glm::radians(pitch)); 
	forward = glm::normalize(front);
}

void Camera::updateFOV(double yOffset)
{
	if(fov > minFov && fov <= maxFov)
		fov -= (float)yOffset;
	if(fov <= minFov)
		fov = minFov;
	if(fov >= maxFov)
		fov = maxFov;
}

void Camera::zoomCamera(double yOffset)
{
	position += (zoomScale * (float)yOffset) * forward;
}
