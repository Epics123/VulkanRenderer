#include "RenderSystem.h"
#include "../Pipeline.h"

RenderSystem::RenderSystem(Device& device)
	: RenderSystemBase(device)
{
}

RenderSystem::~RenderSystem()
{
	RenderSystemBase::~RenderSystemBase();
}

void RenderSystem::init(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout additionalLayout)
{
	textureDescriptorSets.resize(maxDescriptorSets);
	//vertFilePath = "MainApp/resources/vulkan/shaders/TestVert.spv";
	//fragFilePath = "MainApp/resources/vulkan/shaders/TestFrag.spv";
	vertFilePath = "MainApp/resources/vulkan/shaders/PBRVert.spv";
	fragFilePath = "MainApp/resources/vulkan/shaders/PBRFrag.spv";

	RenderSystemBase::init(renderPass, globalSetLayout, additionalLayout);
}

void RenderSystem::update(FrameInfo& frameInfo, Buffer* buffer)
{
	uint32_t i = 0;
	// TODO: Probably shouldn't iterate through every game object a second time but can't think of a better solution atm
	for (auto& keyValue : frameInfo.gameObjects)
	{
		GameObject& obj = keyValue.second;

		if (!obj.model)
			continue;

		ShaderParameters shaderParams = obj.materialComp->material->getShaderParameters();

		MaterialUbo ubo{};
		ubo.albedo = shaderParams.albedo;
		ubo.ambientOcclusion = shaderParams.ambientOcclusion;
		ubo.roughness = shaderParams.roughness;
		ubo.metalic = shaderParams.metallic;

		VkDeviceSize dynamicOffset = static_cast<VkDeviceSize>(i) * frameInfo.dynamicOffset;
		buffer->writeToBuffer(&ubo, buffer->getAlignmentSize(), dynamicOffset);
		buffer->flush(buffer->getBufferSize());
		i++;
	}
}

void RenderSystem::render(FrameInfo& frameInfo)
{
	pipeline->bind(frameInfo.commandBuffer);

	vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);
	
	uint32_t i = 0;
	for (auto& keyValue : frameInfo.gameObjects)
	{
		GameObject& obj = keyValue.second;

		if (!obj.model)
			continue;

		ShaderParameters shaderParams = obj.materialComp->material->getShaderParameters();

		SimplePushConstantData push{};
		push.modelMatrix = obj.transform.getTransform();
		push.normalMatrix = obj.transform.getNormalMatrix();
		push.textureIndex = shaderParams.textureIndex;
		push.toggleTexture = shaderParams.toggleTexture;

		vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);

		if (frameInfo.materialDescriptorSet)
		{
			uint32_t dynamicOffset = static_cast<uint32_t>(i * frameInfo.dynamicOffset);
			vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &frameInfo.materialDescriptorSet, 1, &dynamicOffset);
		}

		obj.model->bind(frameInfo.commandBuffer);
		obj.model->draw(frameInfo.commandBuffer);

		i++;
	}
}

void RenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout additionalLayout /*= VK_NULL_HANDLE*/)
{
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(SimplePushConstantData);
	
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts {globalSetLayout};
	if(additionalLayout != VK_NULL_HANDLE)
		descriptorSetLayouts.push_back(additionalLayout);
	
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

void RenderSystem::createPipeline(VkRenderPass renderPass)
{
	assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout!");

	PipelineConfigInfo pipelineConfig{};
	Pipeline::defaultPipelineConfigInfo(pipelineConfig);
	pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = pipelineLayout;
	pipeline = std::make_unique<Pipeline>(device, vertFilePath, fragFilePath, pipelineConfig);
}
