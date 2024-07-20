#pragma once

#include "../Common/Defines.h"

#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <memory>

struct FrameBufferAttachment
{
	VkImage image;
	VkDeviceMemory memory;
	VkImageView view;
};

// DEFERRED_RENDERING_REWORK

class Texture;

struct RenderPassInitInfo
{
	std::shared_ptr<Texture> attachmentTexture;
	VkAttachmentLoadOp loadOp;
	VkAttachmentStoreOp storeOp;
	VkImageLayout layout;
	VkPipelineBindPoint bindPoint;
	std::string name = "";
};

class RenderPass
{
public:
	MOVABLE_ONLY(RenderPass);

	RenderPass(const std::vector<RenderPassInitInfo>& initInfos, const std::vector<std::shared_ptr<Texture>> resolveAttachments, const std::string& name);
	~RenderPass();

	VkRenderPass getRenderPass() const { return renderPass; };

private:
	VkDevice device = VK_NULL_HANDLE;
	VkRenderPass renderPass = VK_NULL_HANDLE;

	std::string debugName = "";
};

// END_DEFERRED_RENDERING_REWORK	

class RenderPass_
{
public:
	RenderPass_();
	~RenderPass_();

	void begin(VkCommandBuffer commandBuffer, int frameIndex = 0);
	void end(VkCommandBuffer commandBuffer);

	void setMaxFramebufferCount(uint32_t count) { maxFramebuffers = count; }
	VkFramebuffer getFramebuffer(int index) { return framebuffers[index]; }

	void setImageFormat(VkFormat format) { imageFormat = format; }
	void setDepthFormat(VkFormat format) { depthFormat = format; }

	void setShouldDestroyColorImage(bool shouldDestroy) { shouldDestroyColorImages = shouldDestroy; }
	void setShouldDestroyDepthImage(bool shouldDestroy) { shouldDestroyDepthImages = shouldDestroy; }

	virtual void createRenderPass(class Context& device, uint32_t passWidth, uint32_t passHeight);
	virtual void createRenderPassFramebuffers(class Context& device, uint32_t framebufferWidth, uint32_t framebufferHeight);
	virtual void createRenderPassSampler(class Context& device);
	virtual void createRenderPassImageViews(class Context& device);

	virtual void cleanup(class Context& device);

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

class DepthPass : public RenderPass_
{
public:
	DepthPass();
	~DepthPass();

	virtual void createRenderPass(class Context& device, uint32_t passWidth, uint32_t passHeight) override;
	virtual void createRenderPassFramebuffers(class Context& device, uint32_t framebufferWidth, uint32_t framebufferHeight) override;
	virtual void createRenderPassSampler(class Context& device) override;

	//virtual void cleanup(class Device& device) override;

	void createDepthImage(class Context& device);

private:
	
};