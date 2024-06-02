#include "RenderSystemBase.h"

#include "../Core/Pipeline.h"

RenderSystemBase::RenderSystemBase(Context& device)
	:device{ device }
{

}

void RenderSystemBase::init(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout additionalLayout /*= VK_NULL_HANDLE*/)
{
	createPipelineLayout(globalSetLayout, additionalLayout);
	createPipeline(renderPass);
}

void RenderSystemBase::update(FrameInfo& frameInfo, Buffer* buffer)
{

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
