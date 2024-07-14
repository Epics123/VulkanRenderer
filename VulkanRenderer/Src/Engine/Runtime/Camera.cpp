#include "Camera.h"
#include "imgui.h"

Camera::Camera(const glm::vec3& pos, const glm::vec3 target, const glm::vec3 up, float near, float far, float aspect)
	:position{pos}, worldUp{up}, cameraTarget{target}, nearClip{near}, farClip{far}, aspectRatio{aspect}
{
	setPerspectiveProjection(fov, aspectRatio, nearClip, farClip);
}

void Camera::move(const glm::vec3& dir, float speed)
{
	isDirty	= true;
	position = position + (dir * speed);
}

void Camera::rotate(const glm::vec2& delta, float speed)
{
	const glm::quat deltaQuat = glm::quat(glm::vec3(-speed * delta.y, speed * delta.x, 0.0f));
	orientation = deltaQuat * orientation;
	orientation = glm::normalize(orientation);

	{
		isDirty = true;
		const glm::mat4 view = glm::mat4_cast(orientation);
		const glm::vec3 dir = -glm::vec3(view[0][2], view[1][2], view[2][2]);
		orientation = glm::quat(glm::lookAt(position, position + dir, getUp()));
	}
}

void Camera::setPosition(const glm::vec3& newPos)
{
	isDirty = true;
	position = newPos;
}

void Camera::updatePositon(int key, float speed)
{
	// Move camera 
	switch (key)
	{
	case GLFW_KEY_W:
		move(getCameraDirection(), speed);
		break;
	case GLFW_KEY_S:
		move(-getCameraDirection(), speed);
		break;
	case GLFW_KEY_A:
		move(getRight(), speed);
		break;
	case GLFW_KEY_D:
		move(getRight(), speed);
		break;
	case GLFW_KEY_E:
		move(-worldUp, speed);
		break;
	case GLFW_KEY_Q:
		move(worldUp, speed);
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
	if(canScroll)
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
	view = invModel;
	invModel = glm::inverse(view);
}

void Camera::setPerspectiveProjection(float fov, float aspectRatio, float near, float far)
{
	view = invModel;
	invModel = glm::inverse(view);
	proj = glm::perspective(glm::radians(fov), aspectRatio, near, far);
	proj[1][1] *= -1;
}

glm::mat4 Camera::getViewMatrix() const
{
	const glm::mat4 trans = glm::translate(glm::mat4(1.0f), position); // might need -position here
	const glm::mat4 rot = glm::mat4_cast(orientation);
	return rot * trans;
}

glm::vec3 Camera::getCameraDirection() const
{
	const auto view = glm::mat4_cast(orientation);
	return glm::vec3(view[0][2], view[1][2], view[2][2]);
}

glm::vec3 Camera::getRight() const
{
	const auto view = glm::mat4_cast(orientation);
	return glm::vec3(view[0][0], view[1][0], view[2][0]);
}

glm::vec3 Camera::getUp() const
{
	return glm::normalize(glm::cross(getRight(), getCameraDirection()));
}

glm::vec3 Camera::getEulerAngles() const
{
	glm::vec3 eulerAngles = glm::eulerAngles(orientation);
	eulerAngles = glm::degrees(eulerAngles);
	return eulerAngles;
}

void Camera::setEulerAngles(const glm::vec3& newDir)
{
	glm::vec3 eulerAngles = glm::radians(newDir);
	orientation = glm::quat(eulerAngles);
	isDirty = true;
}
