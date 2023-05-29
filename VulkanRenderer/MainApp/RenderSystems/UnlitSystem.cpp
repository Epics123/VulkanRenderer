#include "UnlitSystem.h"
#include "../Pipeline.h"

UnlitSystem::UnlitSystem(Device& device)
	:RenderSystem(device)
{

}

void UnlitSystem::init(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout additionalLayout /*= VK_NULL_HANDLE*/)
{
	vertFilePath = "MainApp/resources/vulkan/shaders/BasicUnlitVert.spv";
	fragFilePath = "MainApp/resources/vulkan/shaders/BasicUnlitFrag.spv";

	RenderSystemBase::init(renderPass, globalSetLayout, additionalLayout);
}

void UnlitSystem::render(FrameInfo& frameInfo)
{
	RenderSystem::render(frameInfo);
}

void UnlitSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout additionalLayout /*= VK_NULL_HANDLE*/)
{
	RenderSystem::createPipelineLayout(globalSetLayout, additionalLayout);
}

void UnlitSystem::createPipeline(VkRenderPass renderPass)
{
	RenderSystem::createPipeline(renderPass);
}
