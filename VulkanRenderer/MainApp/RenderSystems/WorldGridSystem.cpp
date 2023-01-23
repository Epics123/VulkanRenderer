#include "WorldGridSystem.h"
#include "../Pipeline.h"

WorldGridSystem::WorldGridSystem(Device& device)
	: device{ device }
{

}

void WorldGridSystem::init(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
{
	createPipelineLayout(globalSetLayout);
	createPipeline(renderPass);
}

void WorldGridSystem::update(FrameInfo& frameInfo, GlobalUbo& ubo)
{

}

void WorldGridSystem::render(FrameInfo& frameInfo, GlobalUbo& ubo)
{
	pipeline->bind(frameInfo.commandBuffer);

	vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);

	vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
}

void WorldGridSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;//&pushConstantRange;

	VkResult res = vkCreatePipelineLayout(device.getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);
	if (res != VK_SUCCESS)
		throw std::runtime_error("Failed to create pipeline layout!");
}

void WorldGridSystem::createPipeline(VkRenderPass renderPass)
{
	assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout!");

	PipelineConfigInfo pipelineConfig{};
	Pipeline::defaultPipelineConfigInfo(pipelineConfig);
	Pipeline::enableAlphaBlending(pipelineConfig);
	pipelineConfig.attributeDescriptions.clear();
	pipelineConfig.bindingDescriptions.clear();
	pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = pipelineLayout;
	pipeline = std::make_unique<Pipeline>(device, "MainApp/resources/vulkan/shaders/WorldGridVert.spv", "MainApp/resources/vulkan/shaders/WorldGridFrag.spv", pipelineConfig);
}

WorldGridSystem::~WorldGridSystem()
{
	vkDestroyPipelineLayout(device.getDevice(), pipelineLayout, nullptr);
}
