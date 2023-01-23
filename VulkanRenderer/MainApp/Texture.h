#pragma once

#include "Device.h"
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

	void createTextureImageView(Device& device);
	void createTextureSampler(Device& device);

	void cleanup(Device& device);

private:
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	uint32_t id;
	uint32_t mipLevel = 0;
};