#ifndef PIPELINE_H
#define PIPELINE_H
#endif

#define GLFW_INCLUDE_VULKAN

#include <glfw3.h>
#include <glfw3native.h>
#include <vector>

// DEFERRED RENDERING REFACTOR
class Context;

struct SetDescriptor
{
	uint32_t set;
	std::vector<VkDescriptorSetLayoutBinding> bindings;
};

struct PipelineViewport
{
	PipelineViewport(const VkExtent2D& extents)
	{
		viewport = fromExtents(extents);
	}

	PipelineViewport() = default;
	PipelineViewport(const PipelineViewport&) = default;
	PipelineViewport& operator=(const PipelineViewport&) = default;

	PipelineViewport(const VkViewport& inViewport) : viewport(inViewport){}

	PipelineViewport& operator=(const VkViewport& inViewport)
	{
		viewport = inViewport;
		return *this;
	}

	PipelineViewport& operator=(const VkExtent2D inExtent)
	{
		viewport = fromExtents(inExtent);
		return *this;
	}

	VkExtent2D toExtent2D()
	{
		return VkExtent2D{ static_cast<uint32_t>(std::abs(viewport.width), static_cast<uint32_t>(std::abs(viewport.height))) };
	}

	VkViewport toViewport() { return viewport; }

private:
	VkViewport fromExtents(const VkExtent2D& extents)
	{
		VkViewport newViewport;
		newViewport.x = 0;
		newViewport.y = 0;
		newViewport.width = extents.width;
		newViewport.height = extents.height;
		newViewport.minDepth = 0.0f;
		newViewport.maxDepth = 1.0f;

		return newViewport;
	}

	VkViewport viewport = {};
};

struct GraphicsPipelineDescriptor
{
	
};

class Pipeline
{
public:
	
};

// END DEFERRED RENDERING REFACTOR

enum PipelineType
{
	PIPELINE_TYPE_DEFAULT,
	PIPELINE_TYPE_DEPTH
};

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

class Pipeline_
{
public:
	Pipeline_() = delete;

	//Pipeline(VkDevice device);
	Pipeline_(Context& device, const std::string& vertFilePath, const std::string& fragFilePath, const PipelineConfigInfo& configInfo, PipelineType type = PIPELINE_TYPE_DEFAULT);

	~Pipeline_();

	Pipeline_(const Pipeline_&) = delete;
	Pipeline_& operator=(const Pipeline_&) = delete;

	void createGraphicsPipeline(const PipelineConfigInfo& configInfo, const std::string& vertFilePath, const std::string& fragFilePath);
	void createDepthPipeline(const PipelineConfigInfo& configInfo, const std::string& vertFilePath);

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
	Context& device;
	VkPipeline graphicsPipeline;

	VkShaderModule vertShaderModule = VK_NULL_HANDLE;
	VkShaderModule fragShaderModule = VK_NULL_HANDLE;
	//

};
