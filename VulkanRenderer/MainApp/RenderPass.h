#pragma once

#include <vulkan/vulkan.h>
#include <vector>

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

	void begin(VkCommandBuffer commandBuffer, bool frameInProgress, int frameIndex = 0);
	void end(VkCommandBuffer commandBuffer, bool frameInProgress);

	void setMaxFramebufferCount(uint32_t count) { maxFramebuffers = count; }
	VkFramebuffer getFramebuffer(int index) { return framebuffers[index]; }

	virtual void createRenderPass(class Device& device);
	virtual void createRenderPassFramebuffers(class Device& device, uint32_t framebufferWidth, uint32_t framebufferHeight);
	virtual void createRenderPassSampler(class Device& device);
	virtual void createRenderPassImageViews(class Device& device);

	virtual void cleanup(class Device& device);

	uint32_t width, height;
	std::vector<VkFramebuffer> framebuffers;
	FrameBufferAttachment color, depth;
	VkRenderPass renderPass;
	VkSampler sampler;
	VkDescriptorImageInfo descriptor;
	VkClearColorValue clearColor = { {0.01f, 0.01f, 0.01f, 1.0f} };
	VkFormat imageFormat, depthFormat;

protected:
	uint32_t maxFramebuffers = 3;
};

class DepthPass : public RenderPass
{
public:
	DepthPass();
	~DepthPass();

	virtual void createRenderPass(class Device& device) override;
	virtual void createRenderPassFramebuffers(class Device& device, uint32_t framebufferWidth, uint32_t framebufferHeight) override;
	virtual void createRenderPassSampler(class Device& device) override;

	virtual void cleanup(class Device& device) override;

	void createDepthImage(class Device& device);

private:
	
};