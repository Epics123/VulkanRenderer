#pragma once

#include "RenderSystem.h"

#include "../Device.h"
#include "../GameObject.h"
#include "../FrameInfo.h"

#include <vector>
#include <memory>


class UnlitSystem : public RenderSystem
{
public:
	UnlitSystem(Device& device);

	virtual void init(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout additionalLayout = VK_NULL_HANDLE) override;
	virtual void render(FrameInfo& frameInfo) override;

private:
	virtual void createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout additionalLayout = VK_NULL_HANDLE) override;
	virtual void createPipeline(VkRenderPass renderPass) override;
};