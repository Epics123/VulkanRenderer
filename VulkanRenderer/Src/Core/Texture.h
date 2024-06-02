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

	void setTextureFormat(VkFormat format) { textureFormat = format; }

	VkDescriptorSet getDescriptorSet() { return descriptorSet; }

	void cleanup(Context& device);

	std::string& getNameInternal() { return nameInternal; }
	void setNameInternal(std::string name) { nameInternal = name; }

private:
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;
	VkFormat textureFormat = VK_FORMAT_R8G8B8A8_SRGB;

	VkDescriptorSet descriptorSet;

	uint32_t id;
	uint32_t mipLevel = 0;

	std::string nameInternal = "";
};