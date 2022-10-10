#ifndef PIPELINE_H
#define PIPELINE_H
#endif

#define GLFW_INCLUDE_VULKAN

#include <glfw3.h>
#include <glfw3native.h>
#include <vector>

#include "Device.h"

struct PipelineConfigInfo
{
	PipelineConfigInfo(const PipelineConfigInfo&) = delete;
	PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

	VkPipelineViewportStateCreateInfo viewportInfo;
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
	VkPipelineRasterizationStateCreateInfo rasterizationInfo;
	VkPipelineMultisampleStateCreateInfo multisampleInfo;
	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	VkPipelineColorBlendStateCreateInfo colorBlendInfo;
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
	std::vector<VkDynamicState> dynamicStateEnables;
	VkPipelineDynamicStateCreateInfo dynamicStateInfo;
	VkPipelineLayout pipelineLayout = nullptr;
	VkRenderPass renderPass = nullptr;
	uint32_t subpass = 0;
};

class Pipeline
{
public:
	Pipeline() = delete;

	//Pipeline(VkDevice device);
	Pipeline(Device& device, const std::string& vertFilePath, const std::string& fragFilePath, const PipelineConfigInfo& configInfo);

	~Pipeline();

	Pipeline(const Pipeline&) = delete;
	Pipeline& operator=(const Pipeline&) = delete;

	void createGraphicsPipeline(const PipelineConfigInfo& configInfo);

	void createShaderModule(const std::vector<char>& code,  VkShaderModule* shaderModule);

	void createDefaultPipeline(VkShaderModule vertShaderModule, VkShaderModule fragShaderModule, VkRenderPass* renderPass, VkPipelineLayout* pipelineLayout, VkDescriptorSetLayout* descriptorSetLayout, VkExtent2D* swapChainExtent);

	static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

	VkPipeline* getPipeline() { return &pipeline; }

	VkPipelineLayout getPipelineLayout() { return pipelineLayout; }

	VkGraphicsPipelineCreateInfo* getPipelineCreateInfo() { return &pipelineInfo; }

	void bindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint);
	void bind(VkCommandBuffer commandBuffer);

	void destroyPipeline();

	void setPolygonMode(VkPolygonMode mode);
	void setVertexAttributeCount(uint32_t count);

private:
	static std::vector<char> readFile(const std::string& filename);

private:
	VkPipeline pipeline;
	VkDevice device_old;

	VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;

	// rework
	Device& device;
	VkPipeline graphicsPipeline;

	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;
	//

	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkDescriptorSetLayout descriptorSetLayout;
	VkExtent2D swapChainImageExtent;

	VkPipelineShaderStageCreateInfo vertShaderStageInfo;
	VkPipelineShaderStageCreateInfo fragShaderStageInfo;

	uint32_t maxVertexAttributes;
	std::vector<VkVertexInputBindingDescription> bindingDescription;
	std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;

	VkPipelineVertexInputStateCreateInfo vertexInputInfo;
	VkPipelineInputAssemblyStateCreateInfo inputAssembly;

	VkViewport viewport;
	VkRect2D scissor;
	VkPipelineViewportStateCreateInfo viewportState;

	VkPipelineRasterizationStateCreateInfo rasterizer;

	VkPipelineMultisampleStateCreateInfo multisampling;
	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	VkPipelineColorBlendStateCreateInfo colorBlending;
	VkPipelineDepthStencilStateCreateInfo depthStencil;

	VkPipelineDynamicStateCreateInfo dynamicState;
	VkPipelineLayoutCreateInfo pipelineLayoutInfo;

	VkGraphicsPipelineCreateInfo pipelineInfo;
};
