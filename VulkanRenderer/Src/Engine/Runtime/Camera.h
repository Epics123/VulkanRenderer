#ifndef CAMERA_H
#define CAMERA_H

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/type_aligned.hpp>
#include <glfw3.h>
#include <glfw3native.h>

struct UniformTransforms
{
	glm::aligned_mat4 model;
	glm::aligned_mat4 view;
	glm::aligned_mat4 projection;
	glm::aligned_mat4 prevViewMat = glm::mat4(1.0);
	glm::aligned_mat4 jitter = glm::mat4(1.0);
};

class Camera
{
public:
	Camera(const glm::vec3& pos = glm::vec3(0.0f, 3.0f, 0.0f), const glm::vec3 target = glm::vec3(0.0f), 
		   const glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f), float near = 0.1f, float far = 4000.0f, float aspect = 1600.0f / 900.0f);

	void move(const glm::vec3& dir, float speed);

	// Rotates the camera based on mouse delta
	void rotate(const glm::vec2& delta, float speed = 4.0f); //TODO: might want a generic rotate if we ever want to rotate the camera without mouse input

	void setPosition(const glm::vec3& newPos);

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

	glm::mat4 getProjectionMatrix() const { return proj; }
	glm::mat4 getViewMatrix() const;

	glm::vec3 getCameraDirection() const;
	glm::vec3 getRight() const;
	glm::vec3 getUp() const;

	glm::vec3 getPosition() const { return position; }
	glm::vec3 getEulerAngles() const;
	void setEulerAngles(const glm::vec3& newDir);

	bool cameraDirty() const { return isDirty; }
	void setDirty(bool dirty) { isDirty = dirty; }

private:
	glm::vec3 position;
	glm::vec3 rotation{};
	glm::quat orientation{};
	glm::quat invOrient = glm::conjugate(orientation);

	glm::vec3 cameraTarget;

	glm::vec3 forward = orientation * glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 up = orientation * glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 right = glm::normalize(glm::cross(forward, up));

	glm::vec3 worldUp = glm::vec3(0.0f, 0.0f, 1.0f);

	glm::mat4 invModel;
	glm::mat4 view;
	glm::mat4 invView;
	glm::mat4 proj;

	float nearClip;
	float farClip;

	float aspectRatio;

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
	bool isDirty = false;
};


#endif // ifndef CAMERA_H
