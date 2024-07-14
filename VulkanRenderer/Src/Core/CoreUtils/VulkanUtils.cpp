#include "VulkanUtils.h"

#include <cassert>

namespace VulkanUtils
{
	VkImageViewType VulkanUtils::imageTypeToImageViewType(VkImageType imageType, VkImageCreateFlags flags, bool multiview)
	{
		switch (imageType)
		{
		case VK_IMAGE_TYPE_1D:
			return multiview ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
		case VK_IMAGE_TYPE_2D:
		{
			if (flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
			{
				return VK_IMAGE_VIEW_TYPE_CUBE;
			}
			return multiview ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
		}
		case VK_IMAGE_TYPE_3D:
			return VK_IMAGE_VIEW_TYPE_3D;
		case VK_IMAGE_TYPE_MAX_ENUM:
			return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
		default:
			break;
		}
		assert(false);
		return VK_IMAGE_VIEW_TYPE_2D;
	}
}