#pragma once

#include "../Device.h"
#include "../FrameInfo.h"

#include <vector>
#include <memory>
#include <string.h>

class RenderSystemBase
{
public:
	RenderSystemBase(Device& device);
	~RenderSystemBase();

	RenderSystemBase(const RenderSystemBase&) = delete;
	RenderSystemBase& operator=(const RenderSystemBase&) = delete;

	virtual void init(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout additionalLayout = VK_NULL_HANDLE);
	virtual void render(FrameInfo& frameInfo) = 0;

	void setVertShaderFilepath(std::string& name) { vertFilePath = name; }
	void setFragShaderFilepath(std::string& name) { fragFilePath = name; }

protected:
	virtual void createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout additionalLayout = VK_NULL_HANDLE) = 0;
	virtual void createPipeline(VkRenderPass renderPass) = 0;

protected:
	Device& device;

	std::unique_ptr<class Pipeline> pipeline;
	VkPipelineLayout pipelineLayout;

	std::string vertFilePath, fragFilePath;

	const uint32_t maxDescriptorSets = 2;
};