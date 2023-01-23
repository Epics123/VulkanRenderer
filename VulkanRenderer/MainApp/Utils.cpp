#include "Utils.h"

#include "Buffer.h"

#include <stb_image.h>

bool Utils::loadImageFromFile(Device& device, const char* filepath, Texture& outTexture)
{
	int width, height, channels;
	stbi_uc* pixels = stbi_load(filepath, &width, &height, &channels, STBI_rgb_alpha);

	if (!pixels)
	{
		printf("Failed to load texture from file: %s", filepath);
		return false;
	}

	void* pixelPtr = pixels;
	VkDeviceSize imageSize = width * height * 4;
	VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;

	Buffer stagingBuffer(device, imageSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	stagingBuffer.map();
	stagingBuffer.writeToBuffer(pixelPtr, imageSize);
	stagingBuffer.unmap();

	stbi_image_free(pixels);

	VkExtent3D imageExtent;
	imageExtent.width = static_cast<uint32_t>(width);
	imageExtent.height = static_cast<uint32_t>(height);
	imageExtent.depth = 1;

	VkImageCreateInfo imgInfo{};
	imgInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imgInfo.imageType = VK_IMAGE_TYPE_2D;
	imgInfo.format = imageFormat;
	imgInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	imgInfo.extent = imageExtent;
	imgInfo.mipLevels = 1;
	imgInfo.arrayLayers = 1;
	imgInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imgInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imgInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imgInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imgInfo.flags = 0;

	device.createImageWithInfo(imgInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, outTexture.getTextureImage(), outTexture.getTextureImageMemory());
	device.transitionImageLayout(outTexture.getTextureImage(), imageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	device.copyBufferToImage(stagingBuffer.getBuffer(), outTexture.getTextureImage(), static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1);
	device.transitionImageLayout(outTexture.getTextureImage(), imageFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	return true;
}
