#pragma once

#include "../GameObject.h"

#include <unordered_map>

struct SceneData
{
	GameObject::Map objects;
	uint32_t objectCount = 0;
	uint32_t materialCount = 0;
	std::string sceneName;
	std::unordered_map<std::string, std::shared_ptr<Material>> materials;
};