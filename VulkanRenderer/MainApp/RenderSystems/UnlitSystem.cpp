#include "UnlitSystem.h"
#include "../Pipeline.h"

UnlitSystem::UnlitSystem(Device& device)
	: device{ device }
{

}

void UnlitSystem::init(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout)
{
	createPipelineLayout(globalSetLayout, textureSetLayout);
	createPipeline(renderPass);
}

void UnlitSystem::renderGameObjects(FrameInfo& frameInfo)
{
	pipeline->bind(frameInfo.commandBuffer);

	vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);
	if (frameInfo.textureDescriptorSet)
	{
		vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &frameInfo.textureDescriptorSet, 0, nullptr);
	}

	for (auto& keyValue : frameInfo.gameObjects)
	{
		GameObject& obj = keyValue.second;

		if (!obj.model)
			continue;

		UnlitPushConstantData push{};
		push.modelMatrix = obj.transform.getTransform();
		push.normalMatrix = obj.transform.getNormalMatrix();

		vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(UnlitPushConstantData), &push);

		obj.model->bind(frameInfo.commandBuffer);
		obj.model->draw(frameInfo.commandBuffer);
	}
}

void UnlitSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout)
{
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(UnlitPushConstantData);

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };
	if (textureSetLayout != VK_NULL_HANDLE)
		descriptorSetLayouts.push_back(textureSetLayout);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

	VkResult res = vkCreatePipelineLayout(device.getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);
	if (res != VK_SUCCESS)
		throw std::runtime_error("Failed to create pipeline layout!");
}

void UnlitSystem::createPipeline(VkRenderPass renderPass)
{
	assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout!");

	PipelineConfigInfo pipelineConfig{};
	Pipeline::defaultPipelineConfigInfo(pipelineConfig);
	pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = pipelineLayout;
	pipeline = std::make_unique<Pipeline>(device, "MainApp/resources/vulkan/shaders/BasicUnlitVert.spv", "MainApp/resources/vulkan/shaders/BasicUnlitFrag.spv", pipelineConfig);
}

UnlitSystem::~UnlitSystem()
{
	vkDestroyPipelineLayout(device.getDevice(), pipelineLayout, nullptr);
}
