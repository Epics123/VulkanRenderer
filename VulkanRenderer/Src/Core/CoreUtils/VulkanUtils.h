#pragma once

#include <vulkan/vulkan.h>

namespace VulkanUtils
{
	VkImageViewType imageTypeToImageViewType(VkImageType imageType, VkImageCreateFlags flags, bool multiview);
}