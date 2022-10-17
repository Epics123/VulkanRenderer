#include "Camera.h"

void Camera::updatePositon(int key, float speed)
{
	// Move camera 
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
		position -= speed * up;
		break;
	case GLFW_KEY_Q:
		position += speed * up;
	}
}

void Camera::updateCameraRotation(float yaw, float pitch, float roll)
{
	this->pitch += pitch * sensitivity;
	this->yaw += yaw * sensitivity;

	//roll += roll * dt;
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

void Camera::updateModel(float dt)
{
	if (yaw > 360.0f || yaw < -360.0f)
		yaw = 0.0f;

	if (pitch > 360.0f || pitch < -360.0f)
		pitch = 0.0f;

	glm::quat qPitch = glm::angleAxis(glm::radians(pitch), glm::vec3(1, 0, 0));
	glm::quat qYaw = glm::angleAxis(glm::radians(yaw), glm::vec3(0, 0, 1));
	glm::quat qRoll = glm::angleAxis(glm::radians(roll), glm::vec3(0, 0, 1));

	glm::quat cameraOrientation = qPitch * qYaw;
	orientation = glm::normalize(glm::slerp(orientation, cameraOrientation, 1 - powf(smoothing, dt)));
	glm::mat4 rotate = glm::mat4_cast(orientation);

	glm::mat4 translate = glm::mat4(1.0f);
	translate = glm::translate(translate, position);

	invOrient = glm::conjugate(orientation);

	// Update camera vectors
	forward = invOrient * glm::vec3(0.0f, 0.0f, 1.0f);
	up = invOrient * glm::vec3(0.0f, 1.0f, 0.0f);
	right = glm::normalize(glm::cross(forward, up));
	rotation = glm::vec3(pitch, yaw, roll);

	invModel = rotate * translate;
}

void Camera::setPerspectiveProjection(float fov, float aspectRatio, float near, float far)
{
	view = invModel;
	proj = glm::perspective(glm::radians(fov), aspectRatio, near, far);
	proj[1][1] *= -1;
}
