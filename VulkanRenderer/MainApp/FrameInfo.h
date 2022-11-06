#pragma once

#include "Camera.h"
#include "GameObject.h"

#include <vulkan/vulkan.h>

#define MAX_LIGHTS 10

struct PointLight
{
	glm::vec4 position{};
	glm::vec4 color{}; // w is light intensity
	alignas(16) float radius;
};

// TEMP FOR TESTING UBOS
struct GlobalUbo
{
	glm::mat4 projection{ 1.0f };
	glm::mat4 view{ 1.0f };

	//glm::vec4 lightPostion{ 0.5f, 0.0f, 2.0f, 0.0f }; // ignore w
	//glm::vec4 lightColor{ 1.0f, 1.0f, 1.0f, 3.0f }; // w is light intensity
	glm::vec4 abmientColor{ 1.0f, 1.0f, 1.0f, 0.1f };
	PointLight pointLights[MAX_LIGHTS];
	int numLights;
};

struct FrameInfo
{
	int frameIndex;
	float frameTime;
	VkCommandBuffer commandBuffer;
	Camera& camera;
	VkDescriptorSet globalDescriptorSet;
	GameObject::Map &gameObjects;
};
