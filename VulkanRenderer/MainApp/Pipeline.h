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
	PipelineConfigInfo() = default;
	PipelineConfigInfo(const PipelineConfigInfo&) = delete;
	PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

	std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

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

	void createGraphicsPipeline(const PipelineConfigInfo& configInfo, const std::string& vertFilePath, const std::string& fragFilePath);

	void createShaderModule(const std::vector<char>& code,  VkShaderModule* shaderModule);

	static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
	static void enableAlphaBlending(PipelineConfigInfo& configInfo);
	static void enableWireframe(PipelineConfigInfo& configInfo);
	static void disableWireframe(PipelineConfigInfo& configInfo);

	void bindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint);
	void bind(VkCommandBuffer commandBuffer);

	void destroyPipeline();

private:
	static std::vector<char> readFile(const std::string& filename);

private:

	// rework
	Device& device;
	VkPipeline graphicsPipeline;

	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;
	//

};
