#pragma once

#include "Camera.h"
#include "GameObject.h"
#include "Enums.h"
#include "Light.h"

#include <vulkan/vulkan.h>

#define MAX_LIGHTS 10

// TEMP FOR TESTING UBOS
struct GlobalUbo
{
	glm::mat4 projection{1.0f};
	glm::mat4 view{1.0f};
	glm::mat4 inverseView{1.0f};

	glm::vec4 abmientColor{ 1.0f, 1.0f, 1.0f, 0.1f };
	PointLight pointLights[MAX_LIGHTS];
	SpotLight spotLights[MAX_LIGHTS];
	int numLights;
};

struct FrameInfo
{
	int frameIndex;
	float frameTime;
	float framerate;
	float deltaTime;
	bool showGrid;
	RenderMode renderMode;
	VkCommandBuffer commandBuffer;
	Camera& camera;
	VkDescriptorSet globalDescriptorSet;
	GameObject::Map &gameObjects;
};
