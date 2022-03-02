#include "Camera.h"

void Camera::updatePositon(int key, float speed)
{
	invOrient = glm::conjugate(orientation);
	
	forward = invOrient * glm::vec3(0.0f, 0.0f, 1.0f);
	cameraUp = invOrient * glm::vec3(0.0f, 1.0f, 0.0f);
	right = glm::normalize(glm::cross(forward, cameraUp));

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
		position -= speed * cameraUp;
		break;
	case GLFW_KEY_Q:
		position += speed * cameraUp;
	}
}

void Camera::updateCameraRotation(float yaw, float pitch, float roll)
{
	this->pitch += pitch * sensitivity;
	this->yaw += yaw * sensitivity;

	//roll += roll * dt;

	/*if(pitch > 89.0f)
		pitch = 89.0f;
	if(pitch < -89.0f)
		pitch = -89.0f;*/

	/*glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.z = sin(glm::radians(pitch));
	forward = glm::normalize(front);*/
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

void Camera::updateView(float dt)
{
	if (yaw > 360.0f || yaw < -360.0f)
		yaw = 0.0f;

	if (pitch > 360.0f || pitch < -360.0f)
		pitch = 0.0f;

	glm::quat qPitch = glm::angleAxis(glm::radians(pitch), glm::vec3(1, 0, 0));
	glm::quat qYaw = glm::angleAxis(glm::radians(yaw), glm::vec3(0, 0, 1));
	glm::quat qRoll = glm::angleAxis(glm::radians(roll), glm::vec3(0, 0, 1));

	glm::quat cameraOrientation = qPitch * qYaw;
	//glm::quat delta = glm::mix(glm::quat(0, 0, 0, 0), cameraOrientation, dt);
	//orientation = glm::normalize(delta) * cameraOrientation;
	orientation = glm::normalize(cameraOrientation);
	glm::mat4 rotate = glm::mat4_cast(orientation);

	glm::mat4 translate = glm::mat4(1.0f);
	translate = glm::translate(translate, position);

	printf("Pitch: %f, Yaw: %f\n", pitch, yaw);

	//printf("%f, %f, %f\n", position.x, position.y, position.z);

	view = rotate * translate;
}
