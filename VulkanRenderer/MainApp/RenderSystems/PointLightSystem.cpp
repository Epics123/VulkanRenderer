#include "PointLightSystem.h"
#include "../Pipeline.h"

#include <map>

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

void PointLightSystem::update(FrameInfo& frameInfo, LightUbo& ubo)
{
	int lightIndex = 0;
	glm::mat4 lightRot = glm::rotate(glm::mat4(1.0f), frameInfo.deltaTime, { 0.0f, 0.0f, 1.0f });

	for (auto& keyValue : frameInfo.gameObjects)
	{
		GameObject& obj = keyValue.second;
		
		if(!obj.pointLight)
			continue;
		
		obj.transform.translation = glm::vec3(lightRot * glm::vec4(obj.transform.translation, (float)obj.pointLight->lightType));

		// copy light info to ubo
		ubo.pointLights[lightIndex].position = glm::vec4(obj.transform.translation, obj.pointLight->lightType);
		ubo.pointLights[lightIndex].color = glm::vec4(obj.pointLight->color, obj.pointLight->intensity);
		ubo.pointLights[lightIndex].radius = obj.transform.scale.x;
		lightIndex++;
	}
	ubo.numLights = lightIndex;
}

void PointLightSystem::render(FrameInfo& frameInfo, LightUbo& ubo)
{
	// Sort Lights
	std::map<float, GameObject::id_t> sortedLights;
	for (auto& keyValue : frameInfo.gameObjects)
	{
		GameObject& obj = keyValue.second;
		if(!obj.pointLight)
			continue;

		glm::vec3 offset = frameInfo.camera.position - obj.transform.translation;
		float distSqr = glm::dot(offset, offset);
		sortedLights[distSqr] = obj.getID();
	}

	pipeline->bind(frameInfo.commandBuffer);

	vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);

	vkCmdDraw(frameInfo.commandBuffer, 6 * ubo.numLights, 1, 0, 0);
}

void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
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

void PointLightSystem::createPipeline(VkRenderPass renderPass)
{
	assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout!");

	PipelineConfigInfo pipelineConfig{};
	Pipeline::defaultPipelineConfigInfo(pipelineConfig);
	Pipeline::enableAlphaBlending(pipelineConfig);
	pipelineConfig.attributeDescriptions.clear();
	pipelineConfig.bindingDescriptions.clear();
	pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = pipelineLayout;
	pipeline = std::make_unique<Pipeline>(device, "MainApp/resources/vulkan/shaders/PointLightVert.spv", "MainApp/resources/vulkan/shaders/PointLightFrag.spv", pipelineConfig);
}