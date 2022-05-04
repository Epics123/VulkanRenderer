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
	glm::mat4 model;
	glm::vec4 pos = glm::vec4(-2.0f, -2.0f, -2.0f, 0.0f);
	glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	float intensity = 0.1f;

	void updateModel();
};

#endif // !LIGHT_H
