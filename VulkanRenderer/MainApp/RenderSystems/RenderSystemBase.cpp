#include "RenderSystemBase.h"

#include "../Pipeline.h"

RenderSystemBase::RenderSystemBase(Device& device)
	:device{ device }
{

}

void RenderSystemBase::init(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout additionalLayout /*= VK_NULL_HANDLE*/)
{
	createPipelineLayout(globalSetLayout, additionalLayout);
	createPipeline(renderPass);
}

RenderSystemBase::~RenderSystemBase()
{
	vkDestroyPipelineLayout(device.getDevice(), pipelineLayout, nullptr);
}
