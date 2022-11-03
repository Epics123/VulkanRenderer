#include "PointLightSystem.h"
#include "../Pipeline.h"

PointLightSystem::PointLightSystem(Device& device)
	: device{ device }
{
}

PointLightSystem::~PointLightSystem()
{
	vkDestroyPipelineLayout(device.getDevice(), pipelineLayout, nullptr);
}

void PointLightSystem::init(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
{
	createPipelineLayout(globalSetLayout);
	createPipeline(renderPass);
}

void PointLightSystem::update(FrameInfo& frameInfo, GlobalUbo& ubo)
{
	int lightIndex = 0;

	for (auto& keyValue : frameInfo.gameObjects)
	{
		GameObject& obj = keyValue.second;
		
		if(!obj.pointLight)
			continue;
		
		obj.transform.translation.y += 0.001f;
		obj.transform.translation.x += 0.005f;

		// copy light info to ubo
		ubo.pointLights[lightIndex].position = glm::vec4(obj.transform.translation, 1.0f);
		ubo.pointLights[lightIndex].color = glm::vec4(obj.color, obj.pointLight->intensity);
		ubo.pointLights[lightIndex].radius = obj.transform.scale.x;
		lightIndex++;
	}
	ubo.numLights = lightIndex;
}

void PointLightSystem::render(FrameInfo& frameInfo, GlobalUbo& ubo)
{
	pipeline->bind(frameInfo.commandBuffer);

	vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);

	vkCmdDraw(frameInfo.commandBuffer, 6 * ubo.numLights, 1, 0, 0);
}

void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
	//VkPushConstantRange pushConstantRange{};
	//pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	//pushConstantRange.offset = 0;
	//pushConstantRange.size = sizeof(SimplePushConstantData);

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

void PointLightSystem::createPipeline(VkRenderPass renderPass)
{
	assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout!");

	PipelineConfigInfo pipelineConfig{};
	Pipeline::defaultPipelineConfigInfo(pipelineConfig);
	pipelineConfig.attributeDescriptions.clear();
	pipelineConfig.bindingDescriptions.clear();
	pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = pipelineLayout;
	pipeline = std::make_unique<Pipeline>(device, "MainApp/resources/vulkan/shaders/PointLightVert.spv", "MainApp/resources/vulkan/shaders/PointLightFrag.spv", pipelineConfig);
}