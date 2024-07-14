#include "Texture.h"

#include "Context.h"
#include "Buffer.h"

#include "Logging/Log.h"
#include "CoreUtils/VulkanUtils.h"

#include "imgui_impl_vulkan.h"
#include "imgui_internal.h"

#include <stb_image.h>
#include <iostream>
#include <stdexcept>

Texture_::Texture_()
{

}

Texture_::~Texture_()
{
	
}

void Texture_::createTextureImageView(Context& device)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = textureImage;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = textureFormat;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(device.getDevice(), &viewInfo, nullptr, &textureImageView) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture image view!");
	}
}

void Texture_::createTextureSampler(Context& device)
{
	// Get physical device properties in order to calculate reasonable values for hardware
	// TODO: break this out, query at beginning of runtime and pass relevant data in
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(device.getRawPhysicalDevice(), &properties);

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

	// Rather than forcing anisotropy could make it conditional depending on device capabilities (would set maxanisotropy to 1.0f)
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

	if (vkCreateSampler(device.getDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create texture sampler");
	}

	descriptorSet = ImGui_ImplVulkan_AddTexture(textureSampler, textureImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

bool Texture_::isStencil() const
{
	return (textureFormat == VK_FORMAT_S8_UINT || textureFormat == VK_FORMAT_D16_UNORM_S8_UINT ||
			textureFormat == VK_FORMAT_D24_UNORM_S8_UINT ||
			textureFormat == VK_FORMAT_D32_SFLOAT_S8_UINT);
}

bool Texture_::isDepth() const
{
	return (textureFormat == VK_FORMAT_D16_UNORM || textureFormat == VK_FORMAT_D16_UNORM_S8_UINT ||
		textureFormat == VK_FORMAT_D24_UNORM_S8_UINT || textureFormat == VK_FORMAT_D32_SFLOAT ||
		textureFormat == VK_FORMAT_D32_SFLOAT_S8_UINT ||
		textureFormat == VK_FORMAT_X8_D24_UNORM_PACK32);
}	

void Texture_::cleanup(Context& device)
{
	vkDestroySampler(device.getDevice(), textureSampler, nullptr);
	vkDestroyImageView(device.getDevice(), textureImageView, nullptr);
	vkDestroyImage(device.getDevice(), textureImage, nullptr);
	vkFreeMemory(device.getDevice(), textureImageMemory, nullptr);
}

Texture::Texture(const Context& inContext, VkDevice device, VkImage inImage, VkFormat inFormat, VkExtent3D inExtents, 
				 uint32_t numLayers, bool isMultiview, const std::string& name)
	:context{inContext}, image{inImage}, format{inFormat}, extents{inExtents}, layerCount{numLayers}, multiview{isMultiview}, debugName{name}
{
	imageView = createImageView(!multiview ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY, format, 1, layerCount, name);
}

Texture::Texture(const Context& inContext, const TextureCreationInfo& createInfo)
	:context{inContext}, vmaAllocator{context.getMemoryAllocator()}, usageFlags{createInfo.usageFlags}, flags{createInfo.flags},
	 imageType{createInfo.type}, format{createInfo.format}, extents{createInfo.extents}, ownsVkImage{true}, mipLevels{createInfo.numMipLevels},
	 layerCount{createInfo.layerCount}, multiview{createInfo.multiview}, generateMips{createInfo.generateMips}, msaaSamples{createInfo.msaaSamples},
	 imageTiling{createInfo.tiling}, debugName{createInfo.name}
{
	debugName = "Image: " + createInfo.name;

	ASSERT(extents.width > 0 && extents.height > 0, "Texture cannot have dimensions equal to 0");
	ASSERT(mipLevels > 0, "Texture must have at least one mip level");

	if(generateMips)
	{
		mipLevels = getMipLevelCount(extents.width, extents.height);
	}

	ASSERT(!(mipLevels > 1 && msaaSamples != VK_SAMPLE_COUNT_1_BIT), "Multisampled images cannot have more than 1 mip level");

	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.flags = flags;
	imageInfo.imageType = imageType;
	imageInfo.format = format;
	imageInfo.extent = extents;
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = layerCount;
	imageInfo.samples = msaaSamples;
	imageInfo.tiling = imageTiling;
	imageInfo.usage = usageFlags;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.pNext = VK_NULL_HANDLE;
	imageInfo.pQueueFamilyIndices = VK_NULL_HANDLE;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	allocInfo.usage = createInfo.memoryFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ? VMA_MEMORY_USAGE_AUTO_PREFER_HOST : VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
	allocInfo.priority = 1.0f;

	VkResult result = vmaCreateImage(vmaAllocator, &imageInfo, &allocInfo, &image, &vmaAllocation, nullptr);
	if(result != VK_SUCCESS)
	{
		CORE_CRITICAL("Failed to create new texture - {0}! Error code: {1}", debugName, result);
		throw std::runtime_error("");
	}

	if(vmaAllocation != nullptr)
	{
		VmaAllocationInfo allocationInfo;
		vmaGetAllocationInfo(vmaAllocator, vmaAllocation, &allocationInfo);
		deviceSize = allocationInfo.size;
	}

	const VkImageViewType imageViewType = VulkanUtils::imageTypeToImageViewType(imageType, flags, multiview);
	viewType = imageViewType;

	imageView = createImageView(imageViewType, format, mipLevels, layerCount, createInfo.name);
}

Texture::~Texture()
{
	for(const auto view : imageViewFramebuffers)
	{
		vkDestroyImageView(context.getDevice(), view.second, nullptr);
	}

	vkDestroyImageView(context.getDevice(), imageView, nullptr);

	if(ownsVkImage)
	{
		vmaDestroyImage(vmaAllocator, image, vmaAllocation);
	}
}

bool Texture::isDepth() const
{
	return (format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || 
			format == VK_FORMAT_D32_SFLOAT ||format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_X8_D24_UNORM_PACK32);
}

bool Texture::isStencil() const
{
	return (format == VK_FORMAT_S8_UINT || format == VK_FORMAT_D16_UNORM_S8_UINT || 
			format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT_S8_UINT);
}

VkImageView Texture::createImageView(VkImageViewType viewType, VkFormat imageFormat, uint32_t numMips, uint32_t layers, const std::string& name)
{
	const VkImageAspectFlags aspectMask = isDepth() ? VK_IMAGE_ASPECT_DEPTH_BIT : (isStencil() ? VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT);
	
	VkImageViewCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.flags = VkImageViewCreateFlags(0);
	createInfo.image = image;
	createInfo.viewType = viewType;
	createInfo.format = imageFormat;

	VkComponentMapping components;
	components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components = components;

	VkImageSubresourceRange subresourceRange;
	subresourceRange.aspectMask = aspectMask;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = numMips;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.layerCount = multiview ? VK_REMAINING_ARRAY_LAYERS : layers;
	createInfo.subresourceRange = subresourceRange;

	VkImageView imageView{VK_NULL_HANDLE};
	VkResult result = vkCreateImageView(context.getDevice(), &createInfo, nullptr, &imageView);
	if(result != VK_SUCCESS)
	{
		CORE_CRITICAL("Failed to create image view! Error code: {0}", result);
		throw std::runtime_error("");
	}

	return imageView;
}

uint32_t Texture::getMipLevelCount(uint32_t textureWidth, uint32_t textureHeight) const
{
	return static_cast<uint32_t>(std::floor(std::log2(std::max(textureWidth, textureHeight)))) + 1;
}
