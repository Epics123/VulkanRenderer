#include "Framebuffer.h"
#include "../Core/Context.h"
#include "RenderPass.h"
#include "Texture.h"

#include "Logging/Log.h"

Framebuffer::Framebuffer(const class Context& context, VkDevice device, VkRenderPass renderPass, const std::vector<std::shared_ptr<class Texture>>& attachments, const std::shared_ptr<class Texture> depthAttachment, const std::shared_ptr<class Texture> stencilAttachment, const std::string& name /*= ""*/)
{
	std::vector<VkImageView> imageViews;
	for (std::shared_ptr<Texture> texture : attachments)
	{
		imageViews.push_back(texture->getTextureImageView());
	}

	if(depthAttachment)
	{
		imageViews.push_back(depthAttachment->getTextureImageView());
	}

	if(stencilAttachment)
	{
		imageViews.push_back(stencilAttachment->getTextureImageView());
	}

	if(imageViews.empty())
	{
		CORE_ERROR("Cannot create framebuffer with no attachments!");
	}
}
