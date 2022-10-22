#pragma once

#include "Model.h"

#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/quaternion.hpp>

#include <memory>

struct TransformComponent
{
	glm::vec3 translation{};
	glm::vec3 scale{1.0f, 1.0f, 1.0f};
	glm::vec3 rotation{};
	
	glm::mat4 getTransform()
	{
		glm::mat4 transate = glm::translate(glm::mat4{1.0f}, translation);

		glm::quat qPitch = glm::angleAxis(rotation.x, glm::vec3(1, 0, 0));
		glm::quat qYaw = glm::angleAxis(rotation.z, glm::vec3(0, 0, 1));
		glm::quat qRoll = glm::angleAxis(rotation.y, glm::vec3(0, 0, 1));

		glm::quat orientation = qPitch * qYaw * qRoll;
		//orientation = glm::normalize(glm::slerp(orientation, cameraOrientation, 1 - powf(smoothing, dt)));
		glm::mat4 rotate = glm::mat4_cast(orientation);

		glm::mat4 scaleMat = glm::scale(transate, scale);

		glm::mat4 transform = rotate * scaleMat * transate;//glm::scale(transform, scale) * rotate;
		return transform;
	}
};

class GameObject
{
public:
	typedef unsigned int id_t;

	static GameObject createGameObject()
	{
		static id_t currentId = 0;
		return GameObject{currentId++};
	}

	GameObject(const GameObject&) = delete;
	GameObject& operator=(const GameObject&) = delete;
	GameObject(GameObject&&) = default;
	GameObject& operator=(GameObject&&) = default;

	const id_t getID() { return id; }

	std::shared_ptr<Model> model{};
	glm::vec3 color{};
	TransformComponent transform{};

private:
	GameObject(id_t objId) : id{objId} {};

	id_t id;
};