#pragma once

#define GLFW_INCLUDE_VULKAN

#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <glfw3.h>
#include <stdexcept>

#include "VertexAttributes.h"
#include "Light.h"

struct Vertex
{
	static const int VERTEX_ATTRIB_COUNT = 4 + Light::LIGHT_ATTRIB_COUNT;
	static const int WIREFRAME_VERT_ATTRIB_COUNT = 1;

	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
	glm::vec3 vertexNormal;
	Light light;

	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDesc{};
		bindingDesc.binding = 0;
		bindingDesc.stride = sizeof(Vertex);
		bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDesc;
	}

	static std::array<VkVertexInputAttributeDescription, VERTEX_ATTRIB_COUNT> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, VERTEX_ATTRIB_COUNT> attributeDescriptions{};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex, vertexNormal);

		attributeDescriptions[4].binding = 0;
		attributeDescriptions[4].location = 4;
		attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[4].offset = offsetof(Light, pos);

		attributeDescriptions[5].binding = 0;
		attributeDescriptions[5].location = 5;
		attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[5].offset = offsetof(Light, diffuse);

		attributeDescriptions[6].binding = 0;
		attributeDescriptions[6].location = 6;
		attributeDescriptions[6].format = VK_FORMAT_R32_SFLOAT;
		attributeDescriptions[6].offset = offsetof(Light, intensity);

		return attributeDescriptions;
	}

	static std::array<VkVertexInputAttributeDescription, WIREFRAME_VERT_ATTRIB_COUNT> getWireframeAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, WIREFRAME_VERT_ATTRIB_COUNT> attributeDescriptions{};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		return attributeDescriptions;
	}
};

struct UniformBufferObject
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 mvp;
};

struct LightUniformBufferObject 
{
	glm::mat4 model;
	Light pointLights[1];
};


typedef std::vector<Vertex> VertexSTDVector;
typedef std::vector<uint32_t> IndexSTDVector;

typedef std::vector<Vertex>::iterator VertexIter;
typedef std::vector<uint32_t>::iterator IndexIter;

struct Buffer
{
	VkBuffer buffer;
	VkDeviceMemory bufferMemory;
	VkBufferCreateInfo bufferInfo{};

	static VkResult createBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory, VkBufferCreateInfo& bufferInfo);
	static uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	virtual void copyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer src, VkBuffer dst, VkDeviceSize size);

	virtual void destroyBuffer(VkDevice device);
	virtual void freeBufferMemory(VkDevice device);
};

struct VertexBuffer : Buffer
{
	VkResult createVertexBuffer(VkDevice device, VertexSTDVector verticies, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
};

struct IndexBuffer : Buffer
{
	VkResult createIndexBuffer(VkDevice device, IndexSTDVector indicies, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
};

struct UniformBuffer : Buffer
{
	VkResult createUniformBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, VkDeviceSize bufferSize);
};