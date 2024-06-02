#include "UnlitSystem.h"
#include "../Core/Pipeline.h"

UnlitSystem::UnlitSystem(Context& device)
	:RenderSystem(device)
{

}

void UnlitSystem::init(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout additionalLayout /*= VK_NULL_HANDLE*/)
{
	vertFilePath = "Src/resources/vulkan/shaders/BasicUnlitVert.spv";
	fragFilePath = "Src/resources/vulkan/shaders/BasicUnlitFrag.spv";

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
