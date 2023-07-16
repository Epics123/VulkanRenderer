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

	void setTextureFormat(VkFormat format) { textureFormat = format; }

	void cleanup(Device& device);

	std::string& getNameInternal() { return nameInternal; }
	void setNameInternal(std::string name) { nameInternal = name; }

private:
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;
	VkFormat textureFormat = VK_FORMAT_R8G8B8A8_SRGB;

	uint32_t id;
	uint32_t mipLevel = 0;

	std::string nameInternal = "";
};