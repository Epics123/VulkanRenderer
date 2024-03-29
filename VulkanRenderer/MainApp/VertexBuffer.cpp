#include "VertexBuffer.h"
#include "CommandBuffer.h"

VkResult VertexBuffer::createVertexBuffer(VkDevice device, VertexSTDVector verticies, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue)
{
	VkDeviceSize bufferSize = sizeof(verticies[0]) * verticies.size();
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	VkResult result = createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
									VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
									stagingBuffer, stagingBufferMemory, bufferInfo);

	void* data;
	result = vkMapMemory(device, stagingBufferMemory, 0, bufferInfo.size, 0, &data);
	memcpy(data, verticies.data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	result = createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory, bufferInfo);

	copyBuffer(device, commandPool, graphicsQueue, stagingBuffer, buffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);

	return result;
}

VkResult OldBuffer::createBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory, VkBufferCreateInfo& bufferInfo)
{
	// Create the vertex buffer
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size; // size of buffer in bytes
	bufferInfo.usage = usage; 
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
	allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);
	result = vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate buffer memory!");
	}

	vkBindBufferMemory(device, buffer, bufferMemory, 0);

	return result;
}

void OldBuffer::copyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer src, VkBuffer dst, VkDeviceSize size)
{
	// TODO: replace this function with single time command buffer function implementation (see texture mapping/images)

	VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);
	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, src, dst, 1, &copyRegion);
	endSingleTimeCommands(graphicsQueue, device, commandBuffer, commandPool);
}

void OldBuffer::destroyBuffer(VkDevice device)
{
	vkDestroyBuffer(device, buffer, nullptr);
}

void OldBuffer::freeBufferMemory(VkDevice device)
{
	vkFreeMemory(device, bufferMemory, nullptr);
}

uint32_t OldBuffer::findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
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

VkResult IndexBuffer::createIndexBuffer(VkDevice device, IndexSTDVector indicies, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue)
{
	VkDeviceSize bufferSize = sizeof(indicies[0]) * indicies.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	VkResult result = createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory, bufferInfo);

	void* data;
	result = vkMapMemory(device, stagingBufferMemory, 0, bufferInfo.size, 0, &data);
	memcpy(data, indicies.data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	result = createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory, bufferInfo);

	copyBuffer(device, commandPool, graphicsQueue, stagingBuffer, buffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);

	return result;
}

VkResult UniformBuffer::createUniformBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, VkDeviceSize bufferSize)
{
	//VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	VkResult result = createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
									VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
									buffer, bufferMemory, bufferInfo);
	return result;
}
