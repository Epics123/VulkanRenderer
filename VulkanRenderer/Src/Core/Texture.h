#pragma once

#include "../Common/Defines.h"

#include <vma/vk_mem_alloc.h>

#include <string>
#include <unordered_map>

class Buffer;
class Context;

struct TextureCreationInfo
{
	VkImageType type;
	VkFormat format; 
	VkImageCreateFlags flags;
	VkImageUsageFlags usageFlags;
	VkExtent3D extents;
	uint32_t numMipLevels;
	uint32_t layerCount;
	VkMemoryPropertyFlags memoryFlags;
	bool generateMips = false;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	std::string name = ""; 
	bool multiview = false;
	VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
};

class Texture final
{
public:
	MOVABLE_ONLY(Texture);

	explicit Texture(const Context& inContext, const TextureCreationInfo& createInfo);

	// To be used with images that have been created elsewhere (i.e swapchains)
	explicit Texture(const Context& inContext, VkDevice device, VkImage inImage, VkFormat inFormat, 
					VkExtent3D inExtents, uint32_t numLayers = 1, bool isMultiview = false, const std::string& name = "");

	~Texture();
	
	bool isDepth() const;
	bool isStencil() const;

	VkFormat getFormat() const { return format; }
	VkSampleCountFlagBits getSampleCount() const { return msaaSamples; }
	VkImageLayout getLayout() const { return layout; }
	VkImageView getImageView() const { return imageView; }
	VkExtent3D getExtents() const { return extents; }

private:
	VkImageView createImageView(VkImageViewType viewType, VkFormat imageFormat, uint32_t numMips, uint32_t layers, const std::string& name);

	uint32_t getMipLevelCount(uint32_t textureWidth, uint32_t textureHeight) const;

private:
	const Context& context;
	VmaAllocator vmaAllocator = nullptr;
	VmaAllocation vmaAllocation = nullptr;

	VkDeviceSize deviceSize = 0;

	VkImageUsageFlags usageFlags = 0;
	VkImageCreateFlags flags = 0;

	VkImageType imageType = VK_IMAGE_TYPE_2D;
	VkImage image = VK_NULL_HANDLE;
	VkImageView imageView = VK_NULL_HANDLE;
	std::unordered_map<uint32_t, VkImageView> imageViewFramebuffers;

	VkFormat format = VK_FORMAT_UNDEFINED;
	VkExtent3D extents;
	VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
	bool ownsVkImage = false;

	uint32_t mipLevels = 1;
	uint32_t layerCount = 1;
	bool multiview = false;
	bool generateMips = false;

	VkImageViewType viewType;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkImageTiling imageTiling = VK_IMAGE_TILING_OPTIMAL;

	std::string debugName;
};


// Old texture class
class Texture_ 
{
public:
	Texture_();
	~Texture_();

	VkImage& getTextureImage() { return textureImage; }
	VkDeviceMemory& getTextureImageMemory() { return textureImageMemory; }
	VkImageView& getTextureImageView() { return textureImageView; }
	VkSampler& getTextureSampler() { return textureSampler; }

	void createTextureImageView(Context& device);
	void createTextureSampler(Context& device);

	VkFormat getTextureFormat() const { return textureFormat; }
	void setTextureFormat(VkFormat format) { textureFormat = format; }

	VkSampleCountFlagBits getSampleCount() const { return msaaSamples; }

	VkImageLayout getLayout() const { return layout; }

	VkDescriptorSet getDescriptorSet() { return descriptorSet; }

	bool isStencil() const;
	bool isDepth() const;

	void cleanup(Context& device);

	std::string& getNameInternal() { return nameInternal; }
	void setNameInternal(std::string name) { nameInternal = name; }

private:
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;
	VkFormat textureFormat = VK_FORMAT_R8G8B8A8_SRGB;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkImageTiling imageTiling = VK_IMAGE_TILING_OPTIMAL;
	VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkDescriptorSet descriptorSet;

	uint32_t id;
	uint32_t mipLevel = 0;

	std::string nameInternal = "";
};