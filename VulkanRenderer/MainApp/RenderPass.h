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

	void begin(VkCommandBuffer commandBuffer, int frameIndex = 0);
	void end(VkCommandBuffer commandBuffer);

	void setMaxFramebufferCount(uint32_t count) { maxFramebuffers = count; }
	VkFramebuffer getFramebuffer(int index) { return framebuffers[index]; }

	void setImageFormat(VkFormat format) { imageFormat = format; }
	void setDepthFormat(VkFormat format) { depthFormat = format; }

	void setShouldDestroyColorImage(bool shouldDestroy) { shouldDestroyColorImages = shouldDestroy; }
	void setShouldDestroyDepthImage(bool shouldDestroy) { shouldDestroyDepthImages = shouldDestroy; }

	virtual void createRenderPass(class Device& device, uint32_t passWidth, uint32_t passHeight);
	virtual void createRenderPassFramebuffers(class Device& device, uint32_t framebufferWidth, uint32_t framebufferHeight);
	virtual void createRenderPassSampler(class Device& device);
	virtual void createRenderPassImageViews(class Device& device);

	virtual void cleanup(class Device& device);

	uint32_t width, height;
	std::vector<VkFramebuffer> framebuffers;
	std::vector<FrameBufferAttachment> colors, depths;
	VkRenderPass renderPass;
	VkSampler sampler = VK_NULL_HANDLE;
	VkDescriptorImageInfo descriptor;
	VkClearColorValue clearColor = { {0.01f, 0.01f, 0.01f, 1.0f} };
	VkFormat imageFormat, depthFormat;

protected:
	uint32_t maxFramebuffers = 3;
	bool shouldDestroyColorImages = true;
	bool shouldDestroyDepthImages = true;
};

class DepthPass : public RenderPass
{
public:
	DepthPass();
	~DepthPass();

	virtual void createRenderPass(class Device& device, uint32_t passWidth, uint32_t passHeight) override;
	virtual void createRenderPassFramebuffers(class Device& device, uint32_t framebufferWidth, uint32_t framebufferHeight) override;
	virtual void createRenderPassSampler(class Device& device) override;

	//virtual void cleanup(class Device& device) override;

	void createDepthImage(class Device& device);

private:
	
};