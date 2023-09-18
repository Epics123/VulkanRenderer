#pragma once

#include "Camera.h"
#include "GameObject.h"
#include "Enums.h"
#include "Light.h"
#include "Texture.h"

#include <vulkan/vulkan.h>

#define MAX_LIGHTS 10

struct GlobalUbo
{
	glm::mat4 projection{1.0f};
	glm::mat4 view{1.0f};
	glm::mat4 inverseView{1.0f};
};

struct LightUbo
{
	glm::vec4 abmientColor{ 1.0f, 1.0f, 1.0f, 0.1f };
	PointLight pointLights[MAX_LIGHTS];
	SpotLight spotLights[MAX_LIGHTS];
	DirectionalLight directionalLight;
	uint32_t numLights;
	uint32_t numSpotLights;
};

struct MaterialUbo
{
	glm::vec4 albedo;
	alignas(16) float roughness;
	float ambientOcclusion;
	float metalic;
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
	VkDescriptorSet materialDescriptorSet;
	GameObject::Map &gameObjects;
	VkDeviceSize dynamicOffset;
	uint32_t numObjs;
};
