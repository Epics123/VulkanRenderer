#pragma once

#include "GameObject.h"

#include <string>

class SceneSerializer
{
public:
	void serialize(const std::string& filepath, GameObject::Map& gameObjects);
};