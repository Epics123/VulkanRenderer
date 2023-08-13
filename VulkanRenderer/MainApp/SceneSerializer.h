#pragma once

#include "Scene/Scene.h"

#include <string>

class SceneSerializer
{
public:
	void serialize(const std::string& filepath, GameObject::Map& gameObjects);
	bool deserialize(const std::string& filepath, class Device& device, SceneData& outSceneData);
};