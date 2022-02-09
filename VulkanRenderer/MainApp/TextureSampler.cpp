#include "TextureSampler.h"

#include <stdexcept>

// Currently does all sampling linearly
void createTextureSampler(VkDevice& device, VkPhysicalDevice&physicalDevice, VkSampler&sampler)
{
	// Get physical device properties in order to calculate reasonable values for hardware
	// TODO: break this out, query at beginning of runtime and pass relevant data in
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
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;	// Can not be an arbitrary colour

	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

	// False = 0,1 UV, True = 0,texwidth/height
	samplerInfo.unnormalizedCoordinates = VK_FALSE;

	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	if (vkCreateSampler(device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create texture sampler");
	}
}