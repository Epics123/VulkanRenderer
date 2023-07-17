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

void RenderSystemBase::cleanup()
{
	vkDestroyPipelineLayout(device.getDevice(), pipelineLayout, nullptr);
	pipeline.reset();
}

RenderSystemBase::~RenderSystemBase()
{
	//vkDestroyPipelineLayout(device.getDevice(), pipelineLayout, nullptr);
}
