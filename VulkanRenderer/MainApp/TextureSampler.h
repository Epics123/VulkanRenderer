#ifndef TEXTURE_SAMPLER_H
#define TEXTURE_SAMPLER_H

#define GLFW_INCLUDE_VULKAN

#include <glfw3.h>
#include <glfw3native.h>

void createTextureSampler(VkDevice& device, VkPhysicalDevice& physicalDevice, VkSampler&sampler);

#endif // !TEXTURE_SAMPLER_H