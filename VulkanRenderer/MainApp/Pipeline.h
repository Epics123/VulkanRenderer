#ifndef PIPELINE_H
#define PIPELINE_H
#endif

#define GLFW_INCLUDE_VULKAN

#include <glfw3.h>
#include <glfw3native.h>

class Pipeline
{
public:
	Pipeline();

	Pipeline(VkDevice device);

	void createDefaultPipeline(VkShaderModule vertShaderModule, VkShaderModule fragShaderModule, 
								VkRenderPass* renderPass, VkPipelineLayout* pipelineLayout, 
								VkDescriptorSetLayout* descriptorSetLayout, VkExtent2D* swapChainExtent);

	VkPipeline* getPipeline() { return &pipeline; }

	VkGraphicsPipelineCreateInfo* getPipelineCreateInfo() { return &pipelineInfo; }

	void setPolygonMode(VkPolygonMode mode);

private:
	VkPipeline pipeline;
	VkDevice device;

	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkDescriptorSetLayout descriptorSetLayout;
	VkExtent2D swapChainImageExtent;

	VkPipelineShaderStageCreateInfo vertShaderStageInfo;
	VkPipelineShaderStageCreateInfo fragShaderStageInfo;

	VkVertexInputBindingDescription bindingDescription;

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
