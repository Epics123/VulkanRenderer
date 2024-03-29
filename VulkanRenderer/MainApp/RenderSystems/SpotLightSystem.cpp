#include "SpotLightSystem.h"
#include "../Pipeline.h"

SpotLightSystem::SpotLightSystem(Device& device)
	:device {device}
{

}

SpotLightSystem::~SpotLightSystem()
{
	vkDestroyPipelineLayout(device.getDevice(), pipelineLayout, nullptr);
}

void SpotLightSystem::init(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
{
	createPipelineLayout(globalSetLayout);
	createPipeline(renderPass);
}

void SpotLightSystem::update(FrameInfo& frameInfo, LightUbo& ubo)
{
	int lightIndex = 0;

	for (auto& keyValue : frameInfo.gameObjects)
	{
		GameObject& obj = keyValue.second;

		if(obj.directionalLight)
		{
			ubo.directionalLight.position = glm::vec4(obj.transform.translation, obj.directionalLight->lightType);
			ubo.directionalLight.color = glm::vec4(obj.directionalLight->color, obj.directionalLight->intensity);

			glm::vec3 direction = obj.directionalLight->direction;
			ubo.directionalLight.direction = glm::vec4(direction, 0.0f);
		}

		if (!obj.spotLight)
			continue;

		obj.transform.updateTransform();
		// copy light info to ubo
		ubo.spotLights[lightIndex].position = glm::vec4(obj.transform.translation, obj.spotLight->lightType);
		ubo.spotLights[lightIndex].color = glm::vec4(obj.spotLight->color, obj.spotLight->intensity);

		glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f) * obj.transform.orientation;
		ubo.spotLights[lightIndex].direction = glm::vec4(direction, glm::cos(glm::radians(obj.spotLight->cutoffAngle)));
		ubo.spotLights[lightIndex].outerCutoff = glm::cos(glm::radians(obj.spotLight->outerCutoffAngle));

		lightIndex++;
	}
	ubo.numSpotLights = lightIndex;
}

void SpotLightSystem::render(FrameInfo& frameInfo, LightUbo& ubo)
{
	pipeline->bind(frameInfo.commandBuffer);

	vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);

	vkCmdDraw(frameInfo.commandBuffer, 6 * ubo.numSpotLights, 1, 0, 0);
}

void SpotLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	VkResult res = vkCreatePipelineLayout(device.getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);
	if (res != VK_SUCCESS)
		throw std::runtime_error("Failed to create pipeline layout!");
}

void SpotLightSystem::createPipeline(VkRenderPass renderPass)
{
	assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout!");

	PipelineConfigInfo pipelineConfig{};
	Pipeline::defaultPipelineConfigInfo(pipelineConfig);
	Pipeline::enableAlphaBlending(pipelineConfig);
	pipelineConfig.attributeDescriptions.clear();
	pipelineConfig.bindingDescriptions.clear();
	pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = pipelineLayout;
	pipeline = std::make_unique<Pipeline>(device, "MainApp/resources/vulkan/shaders/SpotLightVert.spv", "MainApp/resources/vulkan/shaders/SpotLightFrag.spv", pipelineConfig);
}
