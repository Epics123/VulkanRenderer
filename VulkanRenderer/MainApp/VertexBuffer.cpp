#include "VertexBuffer.h"

VkResult VertexBuffer::createBuffer(VkDevice device, VkPhysicalDevice physicalDevice)
{
	// Create the vertex buffer
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = sizeof(verticies[0]) * verticies.size(); // size of buffer in bytes
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; // this buffer will be used as a vertex buffer
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult result = vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create vertex buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	// Allocate memory to the vertex buffer
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	result = vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate vertex buffer memory!");
	}

	vkBindBufferMemory(device, buffer, bufferMemory, 0);

	void* data;
	result = vkMapMemory(device, bufferMemory, 0, bufferInfo.size, 0, &data);
	memcpy(data, verticies.data(), (size_t)bufferInfo.size);
	vkUnmapMemory(device, bufferMemory);

	return result;
}

void VertexBuffer::destroyBuffer(VkDevice device)
{
	vkDestroyBuffer(device, buffer, nullptr);
}

void VertexBuffer::freeBufferMemory(VkDevice device)
{
	vkFreeMemory(device, bufferMemory, nullptr);
}

uint32_t VertexBuffer::findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties); // Query GPU for available memory types

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		// Check index of memory types and determine if it is suitable
		if((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	}

	throw std::runtime_error("Failed to find suitable memory type!");
}
