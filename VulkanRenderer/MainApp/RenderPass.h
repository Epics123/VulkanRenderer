#pragma once

#include <vulkan/vulkan.h>

struct FrameBufferAttachment
{
	VkImage image;
	VkDeviceMemory memory;
	VkImageView view;
};

class RenderPass
{
public:
	RenderPass();
	~RenderPass();

	virtual void createRenderPass(class Device& device);
	virtual void createRenderPassFramebuffer(class Device& device, uint32_t framebufferWidth, uint32_t framebufferHeight);
	virtual void createRenderPassSampler(class Device& device);

	virtual void cleanup(class Device& device);

	uint32_t width, height;
	VkFramebuffer framebuffer;
	FrameBufferAttachment color, depth;
	VkRenderPass renderPass;
	VkSampler sampler;
	VkDescriptorImageInfo descriptor;
};

class DepthPass : public RenderPass
{
public:
	DepthPass();
	~DepthPass();

	virtual void createRenderPass(class Device& device) override;
	virtual void createRenderPassFramebuffer(class Device& device, uint32_t framebufferWidth, uint32_t framebufferHeight) override;
	virtual void createRenderPassSampler(class Device& device) override;

	virtual void cleanup(class Device& device) override;

private:
	VkFormat depthFormat;
};