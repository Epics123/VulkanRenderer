#include "ShadowSystem.h"
#include "../Pipeline.h"

ShadowSystem::ShadowSystem(Device& device)
	:RenderSystem(device)
{
	
}

void ShadowSystem::init(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout additionalLayout /*= VK_NULL_HANDLE*/)
{
	vertFilePath = "MainApp/resources/vulkan/shaders/ShadowsVert.spv";
	//fragFilePath = "MainApp/resources/vulkan/shaders/PBRFrag.spv";

	RenderSystemBase::init(renderPass, globalSetLayout, additionalLayout);
}

void ShadowSystem::render(FrameInfo& frameInfo)
{
	pipeline->bind(frameInfo.commandBuffer);

	vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);

	for (auto& keyValue : frameInfo.gameObjects)
	{
		GameObject& obj = keyValue.second;

		if (!obj.model)
			continue;

		SimplePushConstantData push{};
		push.modelMatrix = obj.transform.getTransform();
		push.normalMatrix = obj.transform.getNormalMatrix();

		vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);

		obj.model->bind(frameInfo.commandBuffer);
		obj.model->draw(frameInfo.commandBuffer);
	}
}

void ShadowSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout additionalLayout /*= VK_NULL_HANDLE*/)
{
	RenderSystem::createPipelineLayout(globalSetLayout, additionalLayout);
}

void ShadowSystem::createPipeline(VkRenderPass renderPass)
{
	assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout!");

	PipelineConfigInfo pipelineConfig{};
	Pipeline::defaultPipelineConfigInfo(pipelineConfig);
	pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = pipelineLayout;
	pipeline = std::make_unique<Pipeline>(device, vertFilePath, fragFilePath, pipelineConfig, PIPELINE_TYPE_DEPTH); // TODO: Create new function in Pipeline to create a pipeline mean for only depth output
}
