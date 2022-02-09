#include "TextureSampler.h"

#define GLFW_INCLUDE_VULKAN

#include <glfw3.h>
#include <glfw3native.h>

// Currently does all sampling linearly
void createTextureSampler(VkPhysicalDevice&physicalDevice)
{
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

	// Choices for filters are VK_FILTER_LINEAR and VK_FILTER_NEAREST
	samplerInfo.magFilter = VK_FILTER_LINEAR;	// How to intepret magnified texels
	samplerInfo.minFilter = VK_FILTER_LINEAR;	// How to intepret minified texels

	// Sampling mode for 0 > UVW > 1
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 
}