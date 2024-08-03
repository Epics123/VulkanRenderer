#include "Pipeline.h"
#include "Context.h"
#include "VertexBuffer.h"
#include "Light.h"
#include "Model.h"
#include "ShaderModule.h"

#include "Utils/Utils.h"

#include <array>
#include <fstream>
#include <cassert>

Pipeline_::Pipeline_(Context& device, const std::string& vertFilePath, const std::string& fragFilePath, const PipelineConfigInfo& configInfo, PipelineType type)
	:device(device)
{
	switch (type)
	{
	case PIPELINE_TYPE_DEFAULT:
		createGraphicsPipeline(configInfo, vertFilePath, fragFilePath);
		break;
	case PIPELINE_TYPE_DEPTH:
		createDepthPipeline(configInfo, vertFilePath);
		break;
	default:
		break;
	}
	
}

Pipeline_::~Pipeline_()
{
	if(vertShaderModule != VK_NULL_HANDLE)
		vkDestroyShaderModule(device.getDevice(), vertShaderModule, nullptr);
	if(fragShaderModule != VK_NULL_HANDLE)
		vkDestroyShaderModule(device.getDevice(), fragShaderModule, nullptr);
	vkDestroyPipeline(device.getDevice(), graphicsPipeline, nullptr);
}

void Pipeline_::createGraphicsPipeline(const PipelineConfigInfo& configInfo, const std::string& vertFilePath, const std::string& fragFilePath)
{
	assert(configInfo.pipelineLayout != VK_NULL_HANDLE && "Cannot create graphics pipeline: no pipelineLayout provided in configInfo");
	assert(configInfo.renderPass != VK_NULL_HANDLE && "Cannot create graphics pipeline: no renderPass provided in configInfo");

	std::vector<char> vertShaderCode = readFile(vertFilePath);
	std::vector<char> fragShaderCode = readFile(fragFilePath);

	createShaderModule(vertShaderCode, &vertShaderModule);
	createShaderModule(fragShaderCode, &fragShaderModule);

	VkPipelineShaderStageCreateInfo shaderStages[2];
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = vertShaderModule;
	shaderStages[0].pName = "main";
	shaderStages[0].flags = 0;
	shaderStages[0].pNext = nullptr;
	shaderStages[0].pSpecializationInfo = nullptr;
	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = fragShaderModule;
	shaderStages[1].pName = "main";
	shaderStages[1].flags = 0;
	shaderStages[1].pNext = nullptr;
	shaderStages[1].pSpecializationInfo = nullptr;

	std::vector<VkVertexInputBindingDescription> bindingDescriptions = configInfo.bindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions = configInfo.attributeDescriptions;
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
	pipelineInfo.pViewportState = &configInfo.viewportInfo;
	pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
	pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
	pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
	pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
	pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;

	pipelineInfo.layout = configInfo.pipelineLayout;
	pipelineInfo.renderPass = configInfo.renderPass;
	pipelineInfo.subpass = configInfo.subpass;

	pipelineInfo.basePipelineIndex = -1;
	pipelineInfo.basePipelineHandle = nullptr;

	VkResult result = vkCreateGraphicsPipelines(device.getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create graphics pipeline!");
	}
}

void Pipeline_::createDepthPipeline(const PipelineConfigInfo& configInfo, const std::string& vertFilePath)
{
	assert(configInfo.pipelineLayout != VK_NULL_HANDLE && "Cannot create graphics pipeline: no pipelineLayout provided in configInfo");
	assert(configInfo.renderPass != VK_NULL_HANDLE && "Cannot create graphics pipeline: no renderPass provided in configInfo");

	std::vector<char> vertShaderCode = readFile(vertFilePath);

	createShaderModule(vertShaderCode, &vertShaderModule);

	VkPipelineShaderStageCreateInfo shaderStages[1];
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = vertShaderModule;
	shaderStages[0].pName = "main";
	shaderStages[0].flags = 0;
	shaderStages[0].pNext = nullptr;
	shaderStages[0].pSpecializationInfo = nullptr;

	std::vector<VkVertexInputBindingDescription> bindingDescriptions = configInfo.bindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions = configInfo.attributeDescriptions;
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 1;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
	pipelineInfo.pViewportState = &configInfo.viewportInfo;
	pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
	pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
	pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
	pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
	pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;

	pipelineInfo.layout = configInfo.pipelineLayout;
	pipelineInfo.renderPass = configInfo.renderPass;
	pipelineInfo.subpass = configInfo.subpass;

	pipelineInfo.basePipelineIndex = -1;
	pipelineInfo.basePipelineHandle = nullptr;

	VkResult result = vkCreateGraphicsPipelines(device.getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create graphics pipeline!");
	}
}

void Pipeline_::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	if (vkCreateShaderModule(device.getDevice(), &createInfo, nullptr, shaderModule) != VK_SUCCESS)
		throw std::runtime_error("Failed tp create shader module!");
}

void Pipeline_::defaultPipelineConfigInfo(PipelineConfigInfo& configInfo)
{
	configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	configInfo.viewportInfo.viewportCount = 1;
	configInfo.viewportInfo.pViewports = nullptr;
	configInfo.viewportInfo.scissorCount = 1;
	configInfo.viewportInfo.pScissors = nullptr;

	configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
	configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
	configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
	configInfo.rasterizationInfo.lineWidth = 1.0f;
	configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
	configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
	configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
	configInfo.rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
	configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional

	configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
	configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	configInfo.multisampleInfo.minSampleShading = 1.0f;           // Optional
	configInfo.multisampleInfo.pSampleMask = nullptr;             // Optional
	configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
	configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional

	configInfo.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
	configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
	configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
	configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
	configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
	configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
	configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

	configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
	configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
	configInfo.colorBlendInfo.attachmentCount = 1;
	configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
	configInfo.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
	configInfo.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
	configInfo.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
	configInfo.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

	configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
	configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
	configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	configInfo.depthStencilInfo.minDepthBounds = 0.0f;  // Optional
	configInfo.depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
	configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
	configInfo.depthStencilInfo.front = {};  // Optional
	configInfo.depthStencilInfo.back = {};   // Optional

	configInfo.dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
	configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
	configInfo.dynamicStateInfo.flags = 0;

	configInfo.bindingDescriptions = Model::Vertex::getBindingDescriptions();
	configInfo.attributeDescriptions = Model::Vertex::getAttributeDescriptions();
}

void Pipeline_::enableAlphaBlending(PipelineConfigInfo& configInfo)
{
	configInfo.colorBlendAttachment.blendEnable = VK_TRUE;

	configInfo.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
}

void Pipeline_::enableWireframe(PipelineConfigInfo& configInfo)
{
	configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_LINE;
}

void Pipeline_::disableWireframe(PipelineConfigInfo& configInfo)
{
	configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
}

void Pipeline_::bindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint)
{
	vkCmdBindPipeline(commandBuffer, pipelineBindPoint, graphicsPipeline);
}

void Pipeline_::bind(VkCommandBuffer commandBuffer)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
}

std::vector<char> Pipeline_::readFile(const std::string& filename)
{
	// start reading at end of file and read as a binary file
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!file.is_open())
		throw std::runtime_error("Failed to open file!");

	size_t fileSize = (size_t)file.tellg(); // Get file size from read position
	std::vector<char> buffer(fileSize);

	file.seekg(0); // Return to start of file
	file.read(buffer.data(), fileSize); // Read file
	file.close();

	return buffer;
}

// DEFERRED RENDERING REWORK

static constexpr int MAX_DESCRIPTOR_SETS = 4096 * 3;

Pipeline::Pipeline(const Context* inContext, const GraphicsPipelineDescriptor& pipelineDesc, VkRenderPass renderPass, const std::string& name)
	:context{inContext}, graphicsPipelineDesc{pipelineDesc}, bindPoint{VK_PIPELINE_BIND_POINT_GRAPHICS}, vkRenderPass{renderPass}, debugName{name}
{
	createGraphicsPipeline();
}

Pipeline::Pipeline(const Context* inContext, const ComputePipelineDescriptor& pipelineDesc, const std::string& name)
	:context{inContext}, computePipelineDesc{pipelineDesc}, bindPoint{VK_PIPELINE_BIND_POINT_COMPUTE}, debugName{name}
{
	createComputePipeline();
}

Pipeline::Pipeline(const Context* inContext, const RayTracingPipelineDescriptor& pipelineDesc, const std::string& name)
	:context{inContext}, rayTracingPipelineDesc{pipelineDesc}, debugName{name}
{
	createRayTracingPipeline();
}

Pipeline::~Pipeline()
{
	const VkDevice device = context->getDevice();

	vkDestroyPipeline(device, pipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyDescriptorPool(device, descriptorPool, nullptr);

	for(const std::pair<uint32_t, DescriptorSet>& set : descriptorSets)
	{
		vkDestroyDescriptorSetLayout(device, set.second.layout, nullptr);
	}
}

void Pipeline::allocateDescriptors(const std::vector<SetAndCount> setAndCount)
{
	if(descriptorPool = VK_NULL_HANDLE)
	{
		initDescriptorPool();
	}

	for(SetAndCount set : setAndCount)
	{
		const uint32_t setIndex = set.set;

		ASSERT(descriptorSets.find(setIndex) != descriptorSets.end(), "This pipeline does not have a set with index " + std::to_string(setIndex));

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptorSets[setIndex].layout;
		allocInfo.pNext = VK_NULL_HANDLE;

		for(uint32_t i = 0; i < set.count; i++)
		{
			VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
			VK_CHECK(vkAllocateDescriptorSets(context->getDevice(), &allocInfo, &descriptorSet), "Failed to allocate descriptor set!");

			descriptorSets[setIndex].sets.push_back(descriptorSet);
		}
	}
}

void Pipeline::updatePushConstant(VkCommandBuffer cmdBuffer, VkShaderStageFlags flags, uint32_t size, const void* data)
{
	vkCmdPushConstants(cmdBuffer, pipelineLayout, flags, 0, size, data);
}

void Pipeline::updateDescriptorSets()
{
	if(!writeDescSets.empty())
	{
		std::unique_lock<std::mutex> mutexLock(mutex);

		vkUpdateDescriptorSets(context->getDevice(), writeDescSets.size(), writeDescSets.data(), 0, nullptr);

		writeDescSets.clear();
		bufferInfo.clear();

		bufferViewInfo.clear();
		imageInfo.clear();
		accelerationStructInfo.clear();
	}
}

void Pipeline::bind(VkCommandBuffer cmdBuffer)
{
	vkCmdBindPipeline(cmdBuffer, bindPoint, pipeline);
	updateDescriptorSets();
}

void Pipeline::createGraphicsPipeline()
{
	const std::vector<VkSpecializationMapEntry>& vertexSpecConstants = graphicsPipelineDesc.vertexSpecConstants;

	VkSpecializationInfo vertexSpecializationInfo{};
	vertexSpecializationInfo.mapEntryCount = static_cast<uint32_t>(vertexSpecConstants.size());
	vertexSpecializationInfo.pMapEntries = vertexSpecConstants.data();
	vertexSpecializationInfo.dataSize = !vertexSpecConstants.empty() ? vertexSpecConstants.back().offset + vertexSpecConstants.back().size : 0;
	vertexSpecializationInfo.pData = graphicsPipelineDesc.vertexSpecializationData;

	const std::vector<VkSpecializationMapEntry>& fragmentSpecConstants = graphicsPipelineDesc.fragmentSpecConstants;

	VkSpecializationInfo fragmentSpecializationInfo{};
	fragmentSpecializationInfo.mapEntryCount = static_cast<uint32_t>(fragmentSpecConstants.size());
	fragmentSpecializationInfo.pMapEntries = fragmentSpecConstants.data();
	fragmentSpecializationInfo.dataSize = !fragmentSpecConstants.empty() ? fragmentSpecConstants.back().offset + fragmentSpecConstants.back().size : 0;
	fragmentSpecializationInfo.pData = graphicsPipelineDesc.fragmentSpecializationData;

	const std::shared_ptr<ShaderModule> vertexShader = graphicsPipelineDesc.vertexShader.lock();
	ASSERT(vertexShader, "Vertex shader's ShaderModule had been destroyed before being used to create a pipeline!");

	const std::shared_ptr<ShaderModule> fragmentShader = graphicsPipelineDesc.fragmentShader.lock();
	ASSERT(fragmentShader, "Fragment shader's ShaderModule had been destroyed before being used to create a pipeline!");

	VkPipelineShaderStageCreateInfo vertexCreateInfo{};
	vertexCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexCreateInfo.stage = vertexShader->getShaderFlags();
	vertexCreateInfo.module = vertexShader->getShaderModule();
	vertexCreateInfo.pName = vertexShader->getEntryPoint().c_str();
	vertexCreateInfo.pSpecializationInfo = !vertexSpecConstants.empty() ? &vertexSpecializationInfo : nullptr;
	vertexCreateInfo.pNext = nullptr;

	VkPipelineShaderStageCreateInfo fragCreateInfo{};
	fragCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragCreateInfo.stage = fragmentShader->getShaderFlags();
	fragCreateInfo.module = fragmentShader->getShaderModule();
	fragCreateInfo.pName = fragmentShader->getEntryPoint().c_str();
	fragCreateInfo.pSpecializationInfo = !fragmentSpecConstants.empty() ? &fragmentSpecializationInfo : nullptr;
	fragCreateInfo.pNext = nullptr;

	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{vertexCreateInfo, fragCreateInfo};

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
	vertexInputCreateInfo.pVertexBindingDescriptions = 0;
	vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;
	vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;
	vertexInputCreateInfo.pNext = VK_NULL_HANDLE;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = graphicsPipelineDesc.primitiveTopology;
	inputAssembly.primitiveRestartEnable = VK_FALSE;
	inputAssembly.pNext = VK_NULL_HANDLE;

	const VkViewport viewport = graphicsPipelineDesc.viewport.toVkViewport();

	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = graphicsPipelineDesc.viewport.toExtent2D();
	
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;
	viewportState.pNext = VK_NULL_HANDLE;

	VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo{};
	rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerCreateInfo.depthClampEnable = VK_FALSE;
	rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizerCreateInfo.cullMode = VkCullModeFlags(graphicsPipelineDesc.cullMode);
	rasterizerCreateInfo.frontFace = graphicsPipelineDesc.frontFace;
	rasterizerCreateInfo.depthBiasEnable = VK_FALSE;
	rasterizerCreateInfo.depthBiasConstantFactor = 0.0f; // Optional
	rasterizerCreateInfo.depthBiasClamp = 0.0f; // Optional
	rasterizerCreateInfo.depthBiasSlopeFactor = 0.0f; // Optional
	rasterizerCreateInfo.lineWidth = 1.0f;
	rasterizerCreateInfo.pNext = VK_NULL_HANDLE;

	VkPipelineMultisampleStateCreateInfo multisampleCreateInfo{};
	multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleCreateInfo.rasterizationSamples = graphicsPipelineDesc.sampleCount;
	multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
	multisampleCreateInfo.minSampleShading = 1.0f; // Optional
	multisampleCreateInfo.pSampleMask = nullptr; // Optional
	multisampleCreateInfo.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampleCreateInfo.alphaToOneEnable = VK_FALSE; // Optional
	multisampleCreateInfo.pNext = VK_NULL_HANDLE;

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;

	if(graphicsPipelineDesc.blendAttachmentStates.size() > 0)
	{
		ASSERT(graphicsPipelineDesc.blendAttachmentStates.size() == graphicsPipelineDesc.colorTextureFormats.size(), "Blend states need to be provided for all color textures");
		
		colorBlendAttachments = graphicsPipelineDesc.blendAttachmentStates;
	}
	else
	{
		VkPipelineColorBlendAttachmentState defaultAttachmentState{};
		defaultAttachmentState.blendEnable = graphicsPipelineDesc.blendEnable;
		defaultAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; // Optional
		defaultAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // Optional
		defaultAttachmentState.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		defaultAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; // Optional
		defaultAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA; // Optional
		defaultAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
		defaultAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		colorBlendAttachments = std::vector<VkPipelineColorBlendAttachmentState>(graphicsPipelineDesc.colorTextureFormats.size(), defaultAttachmentState);
	}

	VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo{};
	colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendCreateInfo.logicOpEnable = VK_FALSE;
	colorBlendCreateInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlendCreateInfo.attachmentCount = uint32_t(colorBlendAttachments.size());
	colorBlendCreateInfo.pAttachments = colorBlendAttachments.data();

	initDescriptorLayout();

	std::vector<VkDescriptorSetLayout> descSetLayouts(descriptorSets.size());
	for(const std::pair<uint32_t, DescriptorSet>& set : descriptorSets)
	{
		descSetLayouts[set.first] = set.second.layout;
	}
	
	pipelineLayout = createPipelineLayout(descSetLayouts, graphicsPipelineDesc.pushConstants);

	VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
	depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilInfo.depthTestEnable= graphicsPipelineDesc.depthTestEnable;
	depthStencilInfo.depthWriteEnable = graphicsPipelineDesc.depthWriteEnable;
	depthStencilInfo.depthCompareOp = graphicsPipelineDesc.depthCompareOperation;
	depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilInfo.stencilTestEnable = VK_TRUE;
	depthStencilInfo.front = {};
	depthStencilInfo.back = {};
	depthStencilInfo.minDepthBounds = 0.0f;
	depthStencilInfo.maxDepthBounds = 1.0f;
	depthStencilInfo.pNext = VK_NULL_HANDLE;

	VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
	dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(graphicsPipelineDesc.dynamicStates.size());
	dynamicStateInfo.pDynamicStates = graphicsPipelineDesc.dynamicStates.data();
	dynamicStateInfo.pNext = VK_NULL_HANDLE;

	VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo{};
	pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	pipelineRenderingCreateInfo.colorAttachmentCount = uint32_t(graphicsPipelineDesc.colorTextureFormats.size());
	pipelineRenderingCreateInfo.pColorAttachmentFormats = graphicsPipelineDesc.colorTextureFormats.data();
	pipelineRenderingCreateInfo.depthAttachmentFormat = graphicsPipelineDesc.depthTextureFormat;
	pipelineRenderingCreateInfo.stencilAttachmentFormat = graphicsPipelineDesc.stencilTextureFormat;
	pipelineRenderingCreateInfo.pNext = VK_NULL_HANDLE;

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext = graphicsPipelineDesc.useDynamicRendering ? &pipelineRenderingCreateInfo : nullptr;
	pipelineInfo.stageCount = uint32_t(shaderStages.size());
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &graphicsPipelineDesc.vertexInputCreateInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizerCreateInfo;
	pipelineInfo.pMultisampleState = &multisampleCreateInfo;
	pipelineInfo.pDepthStencilState = & depthStencilInfo; // Optional
	pipelineInfo.pColorBlendState = &colorBlendCreateInfo;
	pipelineInfo.pDynamicState = &dynamicStateInfo;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = vkRenderPass;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional
	pipelineInfo.pNext = VK_NULL_HANDLE;

	debugName = "Graphics pipeline: " + debugName;

	const std::string errorMsg = "Failed to create " + debugName + " !";
	VK_CHECK(vkCreateGraphicsPipelines(context->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline), errorMsg.c_str());
}

void Pipeline::createComputePipeline()
{
	const std::shared_ptr<ShaderModule> computeShader = computePipelineDesc.computeShader.lock();
	ASSERT(computeShader, "Compute Shader's ShaderMopdule has been destroyed before being used to create a pipeline!");

	const std::vector<VkSpecializationMapEntry>& specConstants = computePipelineDesc.specializationConstants;

	VkSpecializationInfo specializationInfo{};
	specializationInfo.mapEntryCount = static_cast<uint32_t>(specConstants.size());
	specializationInfo.pMapEntries = specConstants.data();
	specializationInfo.dataSize = !specConstants.empty() ? specConstants.back().offset + specConstants.back().size : 0;
	specializationInfo.pData = computePipelineDesc.specializationData;
	
	initDescriptorLayout();

	std::vector<VkDescriptorSetLayout> descSetLayouts;
	for(const std::pair<uint32_t, DescriptorSet>& set : descriptorSets)
	{
		descSetLayouts.push_back(set.second.layout);
	}

	pipelineLayout = createPipelineLayout(descSetLayouts, computePipelineDesc.pushConstants);

	VkPipelineShaderStageCreateInfo shaderStageInfo{};
	shaderStageInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	shaderStageInfo.flags = 0;
	shaderStageInfo.stage = computeShader->getShaderFlags();
	shaderStageInfo.module = computeShader->getShaderModule();
	shaderStageInfo.pName = computeShader->getEntryPoint().c_str();
	shaderStageInfo.pSpecializationInfo = !computePipelineDesc.specializationConstants.empty() ? &specializationInfo : nullptr;
	shaderStageInfo.pNext = nullptr;

	VkComputePipelineCreateInfo computePipelineInfo{};
	computePipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineInfo.flags = 0;
	computePipelineInfo.stage = shaderStageInfo;
	computePipelineInfo.layout = pipelineLayout;
	computePipelineInfo.pNext = VK_NULL_HANDLE;

	debugName = "Compute pipeline: " + debugName;

	const std::string errorMsg = "Failed to create " + debugName + " !";
	VK_CHECK(vkCreateComputePipelines(context->getDevice(), VK_NULL_HANDLE, 1, &computePipelineInfo, nullptr, &pipeline), errorMsg.c_str());
}

void Pipeline::createRayTracingPipeline()
{

}

void Pipeline::initDescriptorLayout()
{
	std::vector<SetDescriptor> sets;
	getSetDescriptorsFromBindPoint(sets);

	constexpr VkDescriptorBindingFlags flagsToEnable = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;

	for (SetDescriptor& set : sets)
	{
		std::vector<VkDescriptorBindingFlags> bindFlags(set.bindings.size(), flagsToEnable);

		VkDescriptorSetLayoutBindingFlagsCreateInfo layoutFlagsInfo{};
		layoutFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		layoutFlagsInfo.pNext = nullptr;
		layoutFlagsInfo.bindingCount = static_cast<uint32_t>(set.bindings.size());
		layoutFlagsInfo.pBindingFlags = bindFlags.data();

		VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo{};
		descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorLayoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
		descriptorLayoutInfo.bindingCount = static_cast<uint32_t>(set.bindings.size());
		descriptorLayoutInfo.pBindings = !set.bindings.empty() ? set.bindings.data() : nullptr;

		VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;

		const std::string errorMsg = "Failed to create descriptor set " + debugName;
		VK_CHECK(vkCreateDescriptorSetLayout(context->getDevice(), &descriptorLayoutInfo, nullptr, &descriptorSetLayout), errorMsg.c_str());

		descriptorSets[set.set].layout = descriptorSetLayout;
	} 
}

void Pipeline::initDescriptorPool()
{
	std::vector<SetDescriptor> sets;
	getSetDescriptorsFromBindPoint(sets);

	std::vector<VkDescriptorPoolSize> poolSizes;
	for(const SetDescriptor& set : sets)
	{
		for (const VkDescriptorSetLayoutBinding& binding : set.bindings)
		{
			poolSizes.push_back({binding.descriptorType, MAX_DESCRIPTOR_SETS});
		}
	}

	VkDescriptorPoolCreateInfo descriptorPoolInfo{};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
	descriptorPoolInfo.maxSets = MAX_DESCRIPTOR_SETS;
	descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	descriptorPoolInfo.pPoolSizes = poolSizes.data();
	descriptorPoolInfo.pNext = VK_NULL_HANDLE;

	VK_CHECK(vkCreateDescriptorPool(context->getDevice(), &descriptorPoolInfo, nullptr, &descriptorPool), "Failed to create descriptor pool!");
}

VkPipelineLayout Pipeline::createPipelineLayout(const std::vector<VkDescriptorSetLayout>& descLayouts, const std::vector<VkPushConstantRange>& pushConstants)
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = (uint32_t)descLayouts.size();
	pipelineLayoutInfo.pSetLayouts = descLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = !pushConstants.empty() ? static_cast<uint32_t>(pushConstants.size()) : 0;
	pipelineLayoutInfo.pPushConstantRanges = !pushConstants.empty() ? pushConstants.data() : nullptr;

	VkPipelineLayout layout = VK_NULL_HANDLE;
	VK_CHECK(vkCreatePipelineLayout(context->getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout), "Failed to create pipeline layout!");

	return pipelineLayout;
}

void Pipeline::getSetDescriptorsFromBindPoint(std::vector<SetDescriptor>& inOutSets)
{
	switch (bindPoint)
	{
	case VK_PIPELINE_BIND_POINT_GRAPHICS:
		inOutSets = graphicsPipelineDesc.setDescriptors;
		break;
	case VK_PIPELINE_BIND_POINT_COMPUTE:
		inOutSets = computePipelineDesc.setDescriptors;
		break;
	case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR:
		inOutSets = rayTracingPipelineDesc.setDescriptors;
		break;
	default:
		break;
	}
}

// END DEFERRED RENDERING REWORK