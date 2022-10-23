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
	
	glm::mat4 getTransform();
	glm::mat3 getNormalMatrix();
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