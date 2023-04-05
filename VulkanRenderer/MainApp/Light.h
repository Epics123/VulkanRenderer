#ifndef LIGHT_H

#define LIGHT_H
#define GLFW_INCLUDE_VULKAN

#include <glm/glm.hpp>
#include <glfw3.h>

struct Light
{
	glm::vec4 position{}; // w is light type
	glm::vec4 color{}; // w is light intensity
};

struct PointLight : Light
{
	alignas(16) float radius;
};

struct SpotLight : Light
{
	glm::vec4 direction{}; // w is cos of cutoff angle
};

#endif // !LIGHT_H
