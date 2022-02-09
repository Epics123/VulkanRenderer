#ifndef IMAGE_H
#define IMAGE_H

#define GLFW_INCLUDE_VULKAN

#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <glfw3.h>
#include <stdexcept>

struct Image
{
	static void createImage(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	// TODO: abstract out find memory type
	static uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	static void transitionImageLayout(VkQueue&graphicsQueue, VkDevice&device, VkCommandPool&commandPool, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
};

#endif // !IMAGE_H