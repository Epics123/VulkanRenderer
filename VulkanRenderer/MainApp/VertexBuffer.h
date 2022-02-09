#pragma once

#define GLFW_INCLUDE_VULKAN

#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <glfw3.h>
#include <stdexcept>

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDesc{};
		bindingDesc.binding = 0;
		bindingDesc.stride = sizeof(Vertex);
		bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDesc;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
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


typedef std::vector<Vertex> VertexSTDVector;
typedef std::vector<uint32_t> IndexSTDVector;

typedef std::vector<Vertex>::iterator VertexIter;
typedef std::vector<uint32_t>::iterator IndexIter;

//const std::vector<Vertex> verticies = 
//{
//	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
//	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
//	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
//	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
//};
//
//const std::vector<uint32_t> indices = 
//{
//	0, 1, 2, 2, 3, 0
//};


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
	VkResult createUniformBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
};