#pragma once

#include "Scene/Scene.h"

#include <string>

class SceneSerializer
{
public:
	void serialize(const std::string& filepath, GameObject::Map& gameObjects);
	bool deserialize(const std::string& filepath, class Device& device, SceneData& outSceneData);
};

class MaterialSerializer
{
public:
	static void serialize(const std::string& filepath, std::shared_ptr<Material> material);
	static Material deserialize(const std::string& filepath, class Device& device);
};