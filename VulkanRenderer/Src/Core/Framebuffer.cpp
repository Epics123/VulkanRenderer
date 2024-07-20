#include "Framebuffer.h"
#include "RenderPass.h"
#include "Texture.h"

#include "Logging/Log.h"

Framebuffer::Framebuffer(VkDevice device, VkRenderPass renderPass, const std::vector<std::shared_ptr<class Texture>>& attachments, 
						 const std::shared_ptr<class Texture> depthAttachment, const std::shared_ptr<class Texture> stencilAttachment, const std::string& name)
{
	std::vector<VkImageView> imageViews;
	for (std::shared_ptr<Texture> texture : attachments)
	{
		imageViews.push_back(texture->getImageView());
	}

	if(depthAttachment)
	{
		imageViews.push_back(depthAttachment->getImageView());
	}

	if(stencilAttachment)
	{
		imageViews.push_back(stencilAttachment->getImageView());
	}

	if(imageViews.empty())
	{
		CORE_CRITICAL("Cannot create framebuffer with no attachments!");
	}
	ASSERT(!imageViews.empty(), "Creating a framebuffer with no attachments is not supported!")

	const uint32_t width = !attachments.empty() ? attachments[0]->getExtents().width : depthAttachment->getExtents().width;
	const uint32_t height = !attachments.empty() ? attachments[0]->getExtents().height : depthAttachment->getExtents().height;

	VkFramebufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass = renderPass;
	createInfo.attachmentCount = static_cast<uint32_t>(imageViews.size());
	createInfo.pAttachments = imageViews.data();
	createInfo.width = width;
	createInfo.height = height;
	createInfo.layers = 1;
	createInfo.pNext = VK_NULL_HANDLE;

	VkResult result = vkCreateFramebuffer(device, &createInfo, nullptr, &framebuffer);
	
	debugName = "Framebuffer: " + name;
}

Framebuffer::~Framebuffer()
{
	vkDestroyFramebuffer(device, framebuffer, nullptr);
}
