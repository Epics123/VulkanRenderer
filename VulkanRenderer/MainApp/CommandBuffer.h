#ifndef COMMAND_BUFFER_H
#define COMMAND_BUFFER_H

#define GLFW_INCLUDE_VULKAN

#include <glfw3.h>
#include <glfw3native.h>

VkCommandBuffer beginSingleTimeCommands(VkDevice& device, VkCommandPool& commandPool);
void endSingleTimeCommands(VkQueue& graphicsQueue, VkDevice& device, VkCommandBuffer commandBuffer, VkCommandPool& commandPool);

#endif // !COMMAND_BUFFER_H
