#pragma once

#include "../Common/Defines.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>
#include <string>

class Framebuffer
{

public:
	MOVABLE_ONLY(Framebuffer);

	Framebuffer(const class Context& context, VkDevice device, VkRenderPass renderPass,
				const std::vector<std::shared_ptr<class Texture_>>& attachments,
				const std::shared_ptr<class Texture_> depthAttachment,
				const std::shared_ptr<class Texture_> stencilAttachment,
				const std::string& name = "");
	~Framebuffer();

	VkFramebuffer getFramebuffer() const { return framebuffer; }

private:
	VkDevice device = VK_NULL_HANDLE;
	VkFramebuffer framebuffer = VK_NULL_HANDLE;
};
