#ifndef PIPELINE_H
#define PIPELINE_H
#endif

#define GLFW_INCLUDE_VULKAN

#include "../Common/Defines.h"

#include <glfw3.h>
#include <glfw3native.h>

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <mutex>

// DEFERRED RENDERING REFACTOR
class Context;
class ShaderModule;

struct SetDescriptor
{
	uint32_t set;
	std::vector<VkDescriptorSetLayoutBinding> bindings;
};

struct SetAndCount
{
	uint32_t set;
	uint32_t count;
	std::string name;
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

	VkViewport toVkViewport() { return viewport; }

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
	GraphicsPipelineDescriptor()
	{
		vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
		vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
	}

	std::vector<SetDescriptor> setDescriptors;
	std::weak_ptr<ShaderModule> vertexShader;
	std::weak_ptr<ShaderModule> fragmentShader;
	std::vector<VkPushConstantRange> pushConstants;
	std::vector<VkDynamicState> dynamicStates;

	std::vector<VkFormat> colorTextureFormats;
	VkFormat depthTextureFormat = VK_FORMAT_UNDEFINED;
	VkFormat stencilTextureFormat = VK_FORMAT_UNDEFINED;

	bool useDynamicRendering = false;
	bool depthTestEnable = true;
	bool depthWriteEnable = true;

	VkCompareOp depthCompareOperation = VK_COMPARE_OP_LESS;

	bool blendEnable = false;
	uint32_t numBlendAttachments = 0;

	VkPrimitiveTopology primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;
	VkCullModeFlagBits cullMode = VK_CULL_MODE_BACK_BIT;
	VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

	PipelineViewport viewport;

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo;

	std::vector<VkSpecializationMapEntry> vertexSpecConstants;
	std::vector<VkSpecializationMapEntry> fragmentSpecConstants;

	void* vertexSpecializationData = nullptr;
	void* fragmentSpecializationData = nullptr;

	std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates;
};

struct ComputePipelineDescriptor
{
	std::vector<SetDescriptor> setDescriptors;
	std::weak_ptr<ShaderModule> computeShader;
	std::vector<VkPushConstantRange> pushConstants;
	std::vector<VkSpecializationMapEntry> specializationConstants;
	void* specializationData = nullptr;
};

struct RayTracingPipelineDescriptor
{
	std::vector<SetDescriptor> setDescriptors;
	std::weak_ptr<ShaderModule> rayGenShader;
	std::vector<std::weak_ptr<ShaderModule>> rayMissShaders;
	std::vector<std::weak_ptr<ShaderModule>> rayClosestHitShaders;
	std::vector<VkPushConstantRange> pushConstants;

	// Add specialization const, but they are needed per shaderModule?
};

class Pipeline
{
public:
	Pipeline(const Context* inContext, const GraphicsPipelineDescriptor& pipelineDesc, VkRenderPass renderPass, const std::string& name = "");
	Pipeline(const Context* inContext, const ComputePipelineDescriptor& pipelineDesc, const std::string& name = "");
	Pipeline(const Context* inContext, const RayTracingPipelineDescriptor& pipelineDesc, const std::string& name = "");

	~Pipeline();

	VkPipeline getPipeline() const { return pipeline; }
	VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }

	bool isValid() const { return pipeline == VK_NULL_HANDLE; }

	void allocateDescriptors(const std::vector<SetAndCount> setAndCount);

	void updatePushConstant(VkCommandBuffer cmdBuffer, VkShaderStageFlags flags, uint32_t size, const void* data);

	void updateDescriptorSets();

	void bind(VkCommandBuffer cmdBuffer);

private:
	void createGraphicsPipeline();
	void createComputePipeline();
	void createRayTracingPipeline();

	void initDescriptorLayout();
	void initDescriptorPool();

	VkPipelineLayout createPipelineLayout(const std::vector<VkDescriptorSetLayout>& descLayouts, const std::vector<VkPushConstantRange>& pushConstants);

	void getSetDescriptorsFromBindPoint(std::vector<SetDescriptor>& inOutSets);

private:
	const Context* context = nullptr;
	GraphicsPipelineDescriptor graphicsPipelineDesc;
	ComputePipelineDescriptor computePipelineDesc;
	RayTracingPipelineDescriptor rayTracingPipelineDesc;

	VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkRenderPass vkRenderPass = VK_NULL_HANDLE;

	std::string debugName;

	struct DescriptorSet
	{
		std::vector<VkDescriptorSet> sets;
		VkDescriptorSetLayout layout = VK_NULL_HANDLE;
	};

	std::unordered_map<uint32_t, DescriptorSet> descriptorSets;
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

	std::list<std::vector<VkDescriptorBufferInfo>> bufferInfo;
	std::list<VkBufferView> bufferViewInfo;
	std::list<std::vector<VkDescriptorImageInfo>> imageInfo;
	std::vector<VkWriteDescriptorSetAccelerationStructureKHR> accelerationStructInfo;
	std::vector<VkWriteDescriptorSet> writeDescSets;
	std::mutex mutex;
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
