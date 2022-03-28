#ifndef PIPELINE_H
#define PIPELINE_H
#endif

#define GLFW_INCLUDE_VULKAN

#include <glfw3.h>
#include <glfw3native.h>
#include <vector>

class Pipeline
{
public:
	Pipeline();

	Pipeline(VkDevice device);

	void createDefaultPipeline(VkShaderModule vertShaderModule, VkShaderModule fragShaderModule, 
								VkRenderPass* renderPass, VkPipelineLayout* pipelineLayout, 
								VkDescriptorSetLayout* descriptorSetLayout, VkExtent2D* swapChainExtent);

	VkPipeline* getPipeline() { return &pipeline; }

	VkPipelineLayout getPipelineLayout() { return pipelineLayout; }

	VkGraphicsPipelineCreateInfo* getPipelineCreateInfo() { return &pipelineInfo; }

	void bindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint);

	void destroyPipeline();

	void setPolygonMode(VkPolygonMode mode);
	void setVertexAttributeCount(uint32_t count);

private:
	VkPipeline pipeline;
	VkDevice device;

	VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;

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
