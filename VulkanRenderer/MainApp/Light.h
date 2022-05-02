#ifndef LIGHT_H

#define LIGHT_H
#define GLFW_INCLUDE_VULKAN

#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <glfw3.h>
#include <stdexcept>

struct Light
{
	alignas(16)glm::mat4 model;
	alignas(16)glm::vec3 pos = glm::vec3(-2.0, -2.0, -2.0);
	alignas(16)glm::vec4 diffuse = glm::vec4(1.0, 1.0, 1.0, 1.0);
	alignas(4)float intensity = 0.1;

	void updateModel();
};

#endif // !LIGHT_H
