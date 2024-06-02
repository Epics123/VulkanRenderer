#pragma once

#include "Context.h"
#include "Buffer.h"

class Texture 
{
public:
	Texture();
	~Texture();

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