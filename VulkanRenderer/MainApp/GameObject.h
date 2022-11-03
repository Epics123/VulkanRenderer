#pragma once

#include "Model.h"

#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/quaternion.hpp>

#include <memory>
#include <unordered_map>

struct TransformComponent
{
	glm::vec3 translation{};
	glm::vec3 scale{1.0f, 1.0f, 1.0f};
	glm::vec3 rotation{};
	
	glm::mat4 getTransform();
	glm::mat3 getNormalMatrix();
};

struct PointLightComponent
{
	float intensity = 1.0f;
};

class GameObject
{
public:
	typedef unsigned int id_t;
	using Map = std::unordered_map<id_t, GameObject>;

	static GameObject createGameObject()
	{
		static id_t currentId = 0;
		return GameObject{currentId++};
	}

	static GameObject makePointLight(float intensity = 1.0f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.0f));

	GameObject(const GameObject&) = delete;
	GameObject& operator=(const GameObject&) = delete;
	GameObject(GameObject&&) = default;
	GameObject& operator=(GameObject&&) = default;

	const id_t getID() { return id; }

	glm::vec3 color{};
	TransformComponent transform{};

	// optional components
	std::shared_ptr<Model> model{};
	std::unique_ptr<PointLightComponent> pointLight = nullptr;

private:
	GameObject(id_t objId) : id{objId} {};

	id_t id;
};